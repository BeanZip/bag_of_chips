add_requires("raylib")
add_rules("mode.debug", "mode.release")

target("potato_chip")
set_kind("binary")
add_files("src/*.c")
add_packages("raylib")