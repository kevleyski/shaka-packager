idf_component_register(SRCS "main.c"
                            "wifi.c"
                            "net.c"
                            "packed_fs.c"
                            "mongoose.c")
component_compile_options(-DMG_ENABLE_LINES)
component_compile_options(-DMG_ENABLE_PACKED_FS)
component_compile_options(-DHTTP_URL="http://0.0.0.0:80")
component_compile_options(-DHTTPS_URL="https://0.0.0.0:443")
component_compile_options(-DMG_ENABLE_MBEDTLS=0) # change to '1' to enable TLS
