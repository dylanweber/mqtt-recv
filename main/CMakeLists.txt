# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()

# Flash SPIFFS Partition
spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
