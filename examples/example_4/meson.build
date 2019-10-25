###################################################################################
#                                                                                 #
# NAME: meson.build                                                               #
#                                                                                 #
# AUTHOR: Mike Karlesky, Mark VanderVoord, Greg Williams.                         #
# WRITTEN BY: Michael Brockus.                                                    #
#                                                                                 #
# License: MIT                                                                    #
#                                                                                 #
###################################################################################

project('example-4', 'c')

hunt_dep = dependency('hunt', fallback : ['hunt', 'hunt_dep'])

subdir('src')
subdir('test')
