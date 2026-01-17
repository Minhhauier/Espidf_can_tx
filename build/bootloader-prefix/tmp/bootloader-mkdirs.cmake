# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/ESP_IDF_5_4_2/frameworks/esp-idf-v5.4.2/components/bootloader/subproject")
  file(MAKE_DIRECTORY "D:/ESP_IDF_5_4_2/frameworks/esp-idf-v5.4.2/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "D:/Baitap/work/firmware/Fw_for_sim/Fw_can_tx/build/bootloader"
  "D:/Baitap/work/firmware/Fw_for_sim/Fw_can_tx/build/bootloader-prefix"
  "D:/Baitap/work/firmware/Fw_for_sim/Fw_can_tx/build/bootloader-prefix/tmp"
  "D:/Baitap/work/firmware/Fw_for_sim/Fw_can_tx/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Baitap/work/firmware/Fw_for_sim/Fw_can_tx/build/bootloader-prefix/src"
  "D:/Baitap/work/firmware/Fw_for_sim/Fw_can_tx/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Baitap/work/firmware/Fw_for_sim/Fw_can_tx/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Baitap/work/firmware/Fw_for_sim/Fw_can_tx/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
