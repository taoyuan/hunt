# ==========================================
#   Hunt Project - A Test Framework for C
#   Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
#   [Released under MIT License. Please refer to license.txt for details]
# ==========================================

class HuntTestRunnerGenerator
  def initialize(options = nil)
    @options = HuntTestRunnerGenerator.default_options
    case options
    when NilClass then @options
    when String   then @options.merge!(HuntTestRunnerGenerator.grab_config(options))
    when Hash     then @options.merge!(options)
    else raise 'If you specify arguments, it should be a filename or a hash of options'
    end
    require_relative 'type_sanitizer'
  end

  def self.default_options
    {
      includes: [],
      defines: [],
      plugins: [],
      framework: :hunt,
      test_prefix: 'test|spec|should',
      mock_prefix: 'Mock',
      mock_suffix: '',
      setup_name: 'setUp',
      teardown_name: 'tearDown',
      test_reset_name: 'resetTest',
      main_name: 'main', # set to :auto to automatically generate each time
      main_export_decl: '',
      cmdline_args: false,
      use_param_tests: false
    }
  end

  def self.grab_config(config_file)
    options = default_options
    unless config_file.nil? || config_file.empty?
      require 'yaml'
      yaml_guts = YAML.load_file(config_file)
      options.merge!(yaml_guts[:hunt] || yaml_guts[:cmock])
      raise "No :hunt or :cmock section found in #{config_file}" unless options
    end
    options
  end

  def run(input_file, output_file, options = nil)
    @options.merge!(options) unless options.nil?

    # pull required data from source file
    source = File.read(input_file)
    source = source.force_encoding('ISO-8859-1').encode('utf-8', replace: nil)
    tests               = find_tests(source)
    headers             = find_includes(source)
    testfile_includes   = (headers[:local] + headers[:system])
    used_mocks          = find_mocks(testfile_includes)
    testfile_includes   = (testfile_includes - used_mocks)
    testfile_includes.delete_if { |inc| inc =~ /(hunt|cmock)/ }

    # build runner file
    generate(input_file, output_file, tests, used_mocks, testfile_includes)

    # determine which files were used to return them
    all_files_used = [input_file, output_file]
    all_files_used += testfile_includes.map { |filename| filename + '.c' } unless testfile_includes.empty?
    all_files_used += @options[:includes] unless @options[:includes].empty?
    all_files_used += headers[:linkonly] unless headers[:linkonly].empty?
    all_files_used.uniq
  end

  def generate(input_file, output_file, tests, used_mocks, testfile_includes)
    File.open(output_file, 'w') do |output|
      create_header(output, used_mocks, testfile_includes)
      create_externs(output, tests, used_mocks)
      create_mock_management(output, used_mocks)
      create_suite_setup(output)
      create_suite_teardown(output)
      create_reset(output, used_mocks)
      create_main(output, input_file, tests, used_mocks)
    end

    return unless @options[:header_file] && !@options[:header_file].empty?

    File.open(@options[:header_file], 'w') do |output|
      create_h_file(output, @options[:header_file], tests, testfile_includes, used_mocks)
    end
  end

  def find_tests(source)
    tests_and_line_numbers = []

    # contains characters which will be substituted from within strings, doing
    # this prevents these characters from interferring with scrubbers
    # @ is not a valid C character, so there should be no clashes with files genuinely containing these markers
    substring_subs = { '{' => '@co@', '}' => '@cc@', ';' => '@ss@', '/' => '@fs@' }
    substring_re = Regexp.union(substring_subs.keys)
    substring_unsubs = substring_subs.invert                   # the inverse map will be used to fix the strings afterwords
    substring_unsubs['@quote@'] = '\\"'
    substring_unsubs['@apos@'] = '\\\''
    substring_unre = Regexp.union(substring_unsubs.keys)
    source_scrubbed = source.clone
    source_scrubbed = source_scrubbed.gsub(/\\"/, '@quote@')   # hide escaped quotes to allow capture of the full string/char
    source_scrubbed = source_scrubbed.gsub(/\\'/, '@apos@')    # hide escaped apostrophes to allow capture of the full string/char
    source_scrubbed = source_scrubbed.gsub(/("[^"\n]*")|('[^'\n]*')/) { |s| s.gsub(substring_re, substring_subs) } # temporarily hide problematic characters within strings
    source_scrubbed = source_scrubbed.gsub(/\/\/(?:.+\/\*|\*(?:$|[^\/])).*$/, '')  # remove line comments that comment out the start of blocks
    source_scrubbed = source_scrubbed.gsub(/\/\*.*?\*\//m, '')                     # remove block comments
    source_scrubbed = source_scrubbed.gsub(/\/\/.*$/, '')                          # remove line comments (all that remain)
    lines = source_scrubbed.split(/(^\s*\#.*$) | (;|\{|\}) /x)                     # Treat preprocessor directives as a logical line. Match ;, {, and } as end of lines
                           .map { |line| line.gsub(substring_unre, substring_unsubs) } # unhide the problematic characters previously removed

    lines.each_with_index do |line, _index|
      # find tests
      next unless line =~ /^((?:\s*TEST_CASE\s*\(.*?\)\s*)*)\s*void\s+((?:#{@options[:test_prefix]}).*)\s*\(\s*(.*)\s*\)/m
      arguments = Regexp.last_match(1)
      name = Regexp.last_match(2)
      call = Regexp.last_match(3)
      params = Regexp.last_match(4)
      args = nil
      if @options[:use_param_tests] && !arguments.empty?
        args = []
        arguments.scan(/\s*TEST_CASE\s*\((.*)\)\s*$/) { |a| args << a[0] }
      end
      tests_and_line_numbers << { test: name, args: args, call: call, params: params, line_number: 0 }
    end
    tests_and_line_numbers.uniq! { |v| v[:test] }

    # determine line numbers and create tests to run
    source_lines = source.split("\n")
    source_index = 0
    tests_and_line_numbers.size.times do |i|
      source_lines[source_index..-1].each_with_index do |line, index|
        next unless line =~ /\s+#{tests_and_line_numbers[i][:test]}(?:\s|\()/
        source_index += index
        tests_and_line_numbers[i][:line_number] = source_index + 1
        break
      end
    end

    tests_and_line_numbers
  end

  def find_includes(source)
    # remove comments (block and line, in three steps to ensure correct precedence)
    source.gsub!(/\/\/(?:.+\/\*|\*(?:$|[^\/])).*$/, '')  # remove line comments that comment out the start of blocks
    source.gsub!(/\/\*.*?\*\//m, '')                     # remove block comments
    source.gsub!(/\/\/.*$/, '')                          # remove line comments (all that remain)

    # parse out includes
    includes = {
      local: source.scan(/^\s*#include\s+\"\s*(.+)\.[hH]\s*\"/).flatten,
      system: source.scan(/^\s*#include\s+<\s*(.+)\s*>/).flatten.map { |inc| "<#{inc}>" },
      linkonly: source.scan(/^TEST_FILE\(\s*\"\s*(.+)\.[cC]\w*\s*\"/).flatten
    }
    includes
  end

  def find_mocks(includes)
    mock_headers = []
    includes.each do |include_path|
      include_file = File.basename(include_path)
      mock_headers << include_path if include_file =~ /^#{@options[:mock_prefix]}.*#{@options[:mock_suffix]}$/i
    end
    mock_headers
  end

  def create_header(output, mocks, testfile_includes = [])
    output.puts('/* AUTOGENERATED FILE. DO NOT EDIT. */')
    create_runtest(output, mocks)
    output.puts("\n/*=======Automagically Detected Files To Include=====*/")
    output.puts('#define HUNT_INCLUDE_SETUP_STUBS') if @options[:suite_setup].nil?
    output.puts("#include \"#{@options[:framework]}.h\"")
    output.puts('#include "cmock.h"') unless mocks.empty?
    output.puts('#ifndef HUNT_EXCLUDE_SETJMP_H')
    output.puts('#include <setjmp.h>')
    output.puts('#endif')
    output.puts('#include <stdio.h>')
    if @options[:defines] && !@options[:defines].empty?
      @options[:defines].each { |d| output.puts("#ifndef #{d}\n#define #{d}\n#endif /* #{d} */") }
    end
    if @options[:header_file] && !@options[:header_file].empty?
      output.puts("#include \"#{File.basename(@options[:header_file])}\"")
    else
      @options[:includes].flatten.uniq.compact.each do |inc|
        output.puts("#include #{inc.include?('<') ? inc : "\"#{inc.gsub('.h', '')}.h\""}")
      end
      testfile_includes.each do |inc|
        output.puts("#include #{inc.include?('<') ? inc : "\"#{inc.gsub('.h', '')}.h\""}")
      end
    end
    mocks.each do |mock|
      output.puts("#include \"#{mock.gsub('.h', '')}.h\"")
    end
    output.puts('#include "CException.h"') if @options[:plugins].include?(:cexception)

    return unless @options[:enforce_strict_ordering]

    output.puts('')
    output.puts('int GlobalExpectCount;')
    output.puts('int GlobalVerifyOrder;')
    output.puts('char* GlobalOrderError;')
  end

  def create_externs(output, tests, _mocks)
    output.puts("\n/*=======External Functions This Runner Calls=====*/")
    output.puts("extern void #{@options[:setup_name]}(void);")
    output.puts("extern void #{@options[:teardown_name]}(void);")
    output.puts("\n#ifdef __cplusplus\nextern \"C\"\n{\n#endif") if @options[:externc]
    tests.each do |test|
      output.puts("extern void #{test[:test]}(#{test[:call] || 'void'});")
    end
    output.puts("#ifdef __cplusplus\n}\n#endif") if @options[:externc]
    output.puts('')
  end

  def create_mock_management(output, mock_headers)
    return if mock_headers.empty?

    output.puts("\n/*=======Mock Management=====*/")
    output.puts('static void CMock_Init(void)')
    output.puts('{')

    if @options[:enforce_strict_ordering]
      output.puts('  GlobalExpectCount = 0;')
      output.puts('  GlobalVerifyOrder = 0;')
      output.puts('  GlobalOrderError = NULL;')
    end

    mocks = mock_headers.map { |mock| File.basename(mock) }
    mocks.each do |mock|
      mock_clean = TypeSanitizer.sanitize_c_identifier(mock)
      output.puts("  #{mock_clean}_Init();")
    end
    output.puts("}\n")

    output.puts('static void CMock_Verify(void)')
    output.puts('{')
    mocks.each do |mock|
      mock_clean = TypeSanitizer.sanitize_c_identifier(mock)
      output.puts("  #{mock_clean}_Verify();")
    end
    output.puts("}\n")

    output.puts('static void CMock_Destroy(void)')
    output.puts('{')
    mocks.each do |mock|
      mock_clean = TypeSanitizer.sanitize_c_identifier(mock)
      output.puts("  #{mock_clean}_Destroy();")
    end
    output.puts("}\n")
  end

  def create_suite_setup(output)
    output.puts("\n/*=======Suite Setup=====*/")
    output.puts('static void suite_setup(void)')
    output.puts('{')
    if @options[:suite_setup].nil?
      # New style, call suiteSetUp() if we can use weak symbols
      output.puts('#if defined(HUNT_WEAK_ATTRIBUTE) || defined(HUNT_WEAK_PRAGMA)')
      output.puts('  suiteSetUp();')
      output.puts('#endif')
    else
      # Old style, C code embedded in the :suite_setup option
      output.puts(@options[:suite_setup])
    end
    output.puts('}')
  end

  def create_suite_teardown(output)
    output.puts("\n/*=======Suite Teardown=====*/")
    output.puts('static int suite_teardown(int num_failures)')
    output.puts('{')
    if @options[:suite_teardown].nil?
      # New style, call suiteTearDown() if we can use weak symbols
      output.puts('#if defined(HUNT_WEAK_ATTRIBUTE) || defined(HUNT_WEAK_PRAGMA)')
      output.puts('  return suiteTearDown(num_failures);')
      output.puts('#else')
      output.puts('  return num_failures;')
      output.puts('#endif')
    else
      # Old style, C code embedded in the :suite_teardown option
      output.puts(@options[:suite_teardown])
    end
    output.puts('}')
  end

  def create_runtest(output, used_mocks)
    cexception = @options[:plugins].include? :cexception
    va_args1   = @options[:use_param_tests] ? ', ...' : ''
    va_args2   = @options[:use_param_tests] ? '__VA_ARGS__' : ''
    output.puts("\n/*=======Test Runner Used To Run Each Test Below=====*/")
    output.puts('#define RUN_TEST_NO_ARGS') if @options[:use_param_tests]
    output.puts("#define RUN_TEST(TestFunc, TestLineNum#{va_args1}) \\")
    output.puts('{ \\')
    output.puts("  Hunt.CurrentTestName = #TestFunc#{va_args2.empty? ? '' : " \"(\" ##{va_args2} \")\""}; \\")
    output.puts('  Hunt.CurrentTestLineNumber = TestLineNum; \\')
    output.puts('  if (HuntTestMatches()) { \\') if @options[:cmdline_args]
    output.puts('  Hunt.NumberOfTests++; \\')
    output.puts('  HUNT_EXEC_TIME_START(); \\')
    output.puts('  CMock_Init(); \\') unless used_mocks.empty?
    output.puts('  HUNT_CLR_DETAILS(); \\') unless used_mocks.empty?
    output.puts('  if (TEST_PROTECT()) \\')
    output.puts('  { \\')
    output.puts('    CEXCEPTION_T e; \\') if cexception
    output.puts('    Try { \\') if cexception
    output.puts("      #{@options[:setup_name]}(); \\")
    output.puts("      TestFunc(#{va_args2}); \\")
    output.puts('    } Catch(e) { TEST_ASSERT_EQUAL_HEX32_MESSAGE(CEXCEPTION_NONE, e, "Unhandled Exception!"); } \\') if cexception
    output.puts('  } \\')
    output.puts('  if (TEST_PROTECT()) \\')
    output.puts('  { \\')
    output.puts("    #{@options[:teardown_name]}(); \\")
    output.puts('    CMock_Verify(); \\') unless used_mocks.empty?
    output.puts('  } \\')
    output.puts('  CMock_Destroy(); \\') unless used_mocks.empty?
    output.puts('  HUNT_EXEC_TIME_STOP(); \\')
    output.puts('  HuntConcludeTest(); \\')
    output.puts('  } \\') if @options[:cmdline_args]
    output.puts("}\n")
  end

  def create_reset(output, used_mocks)
    output.puts("\n/*=======Test Reset Option=====*/")
    output.puts("void #{@options[:test_reset_name]}(void);")
    output.puts("void #{@options[:test_reset_name]}(void)")
    output.puts('{')
    output.puts('  CMock_Verify();') unless used_mocks.empty?
    output.puts('  CMock_Destroy();') unless used_mocks.empty?
    output.puts("  #{@options[:teardown_name]}();")
    output.puts('  CMock_Init();') unless used_mocks.empty?
    output.puts("  #{@options[:setup_name]}();")
    output.puts('}')
  end

  def create_main(output, filename, tests, used_mocks)
    output.puts("\n\n/*=======MAIN=====*/")
    main_name = @options[:main_name].to_sym == :auto ? "main_#{filename.gsub('.c', '')}" : (@options[:main_name]).to_s
    if @options[:cmdline_args]
      if main_name != 'main'
        output.puts("#{@options[:main_export_decl]} int #{main_name}(int argc, char** argv);")
      end
      output.puts("#{@options[:main_export_decl]} int #{main_name}(int argc, char** argv)")
      output.puts('{')
      output.puts('  int parse_status = HuntParseOptions(argc, argv);')
      output.puts('  if (parse_status != 0)')
      output.puts('  {')
      output.puts('    if (parse_status < 0)')
      output.puts('    {')
      output.puts("      HuntPrint(\"#{filename.gsub('.c', '')}.\");")
      output.puts('      HUNT_PRINT_EOL();')
      if @options[:use_param_tests]
        tests.each do |test|
          if test[:args].nil? || test[:args].empty?
            output.puts("      HuntPrint(\"  #{test[:test]}(RUN_TEST_NO_ARGS)\");")
            output.puts('      HUNT_PRINT_EOL();')
          else
            test[:args].each do |args|
              output.puts("      HuntPrint(\"  #{test[:test]}(#{args})\");")
              output.puts('      HUNT_PRINT_EOL();')
            end
          end
        end
      else
        tests.each { |test| output.puts("      HuntPrint(\"  #{test[:test]}\");\n    HUNT_PRINT_EOL();") }
      end
      output.puts('    return 0;')
      output.puts('    }')
      output.puts('  return parse_status;')
      output.puts('  }')
    else
      if main_name != 'main'
        output.puts("#{@options[:main_export_decl]} int #{main_name}(void);")
      end
      output.puts("int #{main_name}(void)")
      output.puts('{')
    end
    output.puts('  suite_setup();')
    output.puts("  HuntBegin(\"#{filename.gsub(/\\/, '\\\\\\')}\");")
    if @options[:use_param_tests]
      tests.each do |test|
        if test[:args].nil? || test[:args].empty?
          output.puts("  RUN_TEST(#{test[:test]}, #{test[:line_number]}, RUN_TEST_NO_ARGS);")
        else
          test[:args].each { |args| output.puts("  RUN_TEST(#{test[:test]}, #{test[:line_number]}, #{args});") }
        end
      end
    else
      tests.each { |test| output.puts("  RUN_TEST(#{test[:test]}, #{test[:line_number]});") }
    end
    output.puts
    output.puts('  CMock_Guts_MemFreeFinal();') unless used_mocks.empty?
    output.puts('  return suite_teardown(HuntEnd());')
    output.puts('}')
  end

  def create_h_file(output, filename, tests, testfile_includes, used_mocks)
    filename = File.basename(filename).gsub(/[-\/\\\.\,\s]/, '_').upcase
    output.puts('/* AUTOGENERATED FILE. DO NOT EDIT. */')
    output.puts("#ifndef _#{filename}")
    output.puts("#define _#{filename}\n\n")
    output.puts("#include \"#{@options[:framework]}.h\"")
    output.puts('#include "cmock.h"') unless used_mocks.empty?
    @options[:includes].flatten.uniq.compact.each do |inc|
      output.puts("#include #{inc.include?('<') ? inc : "\"#{inc.gsub('.h', '')}.h\""}")
    end
    testfile_includes.each do |inc|
      output.puts("#include #{inc.include?('<') ? inc : "\"#{inc.gsub('.h', '')}.h\""}")
    end
    output.puts "\n"
    tests.each do |test|
      if test[:params].nil? || test[:params].empty?
        output.puts("void #{test[:test]}(void);")
      else
        output.puts("void #{test[:test]}(#{test[:params]});")
      end
    end
    output.puts("#endif\n\n")
  end
end

if $0 == __FILE__
  options = { includes: [] }

  # parse out all the options first (these will all be removed as we go)
  ARGV.reject! do |arg|
    case arg
    when '-cexception'
      options[:plugins] = [:cexception]
      true
    when /\.*\.ya?ml/
      options = HuntTestRunnerGenerator.grab_config(arg)
      true
    when /--(\w+)=\"?(.*)\"?/
      options[Regexp.last_match(1).to_sym] = Regexp.last_match(2)
      true
    when /\.*\.h/
      options[:includes] << arg
      true
    else false
    end
  end

  # make sure there is at least one parameter left (the input file)
  unless ARGV[0]
    puts ["\nusage: ruby #{__FILE__} (files) (options) input_test_file (output)",
          "\n  input_test_file         - this is the C file you want to create a runner for",
          '  output                  - this is the name of the runner file to generate',
          '                            defaults to (input_test_file)_Runner',
          '  files:',
          '    *.yml / *.yaml        - loads configuration from here in :hunt or :cmock',
          '    *.h                   - header files are added as #includes in runner',
          '  options:',
          '    -cexception           - include cexception support',
          '    -externc              - add extern "C" for cpp support',
          '    --setup_name=""       - redefine setUp func name to something else',
          '    --teardown_name=""    - redefine tearDown func name to something else',
          '    --main_name=""        - redefine main func name to something else',
          '    --test_prefix=""      - redefine test prefix from default test|spec|should',
          '    --test_reset_name=""  - redefine resetTest func name to something else',
          '    --suite_setup=""      - code to execute for setup of entire suite',
          '    --suite_teardown=""   - code to execute for teardown of entire suite',
          '    --use_param_tests=1   - enable parameterized tests (disabled by default)',
          '    --header_file=""      - path/name of test header file to generate too'].join("\n")
    exit 1
  end

  # create the default test runner name if not specified
  ARGV[1] = ARGV[0].gsub('.c', '_Runner.c') unless ARGV[1]

  HuntTestRunnerGenerator.new(options).run(ARGV[0], ARGV[1])
end
