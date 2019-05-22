#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := doorbell-recv

include $(IDF_PATH)/make/project.mk

# Flash SPIFFS partition
$(eval $(call spiffs_create_partition_image,storage,spiffs_image,FLASH_IN_PROJECT))
