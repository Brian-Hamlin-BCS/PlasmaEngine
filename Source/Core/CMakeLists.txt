add_subdirectory(CodeTranslator)
add_subdirectory(Common)
add_subdirectory(Content)
add_subdirectory(Engine)
add_subdirectory(Gameplay)
add_subdirectory(Geometry)
add_subdirectory(Meta)
add_subdirectory(Serialization)
add_subdirectory(SpatialPartition)
add_subdirectory(Startup)
add_subdirectory(Support)


set_property(TARGET "CodeTranslator" PROPERTY FOLDER "Core")
set_property(TARGET "Common" PROPERTY FOLDER "Core")
set_property(TARGET "Content" PROPERTY FOLDER "Core")
set_property(TARGET "Engine" PROPERTY FOLDER "Core")
set_property(TARGET "Gameplay" PROPERTY FOLDER "Core")
set_property(TARGET "Geometry" PROPERTY FOLDER "Core")
set_property(TARGET "Meta" PROPERTY FOLDER "Core")
set_property(TARGET "Serialization" PROPERTY FOLDER "Core")
set_property(TARGET "SpatialPartition" PROPERTY FOLDER "Core")
set_property(TARGET "Startup" PROPERTY FOLDER "Core")
set_property(TARGET "Support" PROPERTY FOLDER "Core")