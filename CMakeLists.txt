# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr)
project(poc_geocan_nrf52840)

# Add your source file to the "app" target. This must come after
# find_package(Zephyr) which defines the target.
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/include/drivers/)
