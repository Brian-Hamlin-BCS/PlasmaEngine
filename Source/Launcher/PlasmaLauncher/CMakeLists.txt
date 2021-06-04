PlasmaCmakeInit()
# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

PlasmaCreateTarget(LIBRARY ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    assimp
    CodeTranslator
    Common
    Content
    EditorCore
    ExportTool
    Engine
    FreeType
    Gameplay
    Geometry
    GraphicsRuntime
    RendererGL
    Libpng
    Meta
    NetworkCore
    Nvtt
    Opus
    Physics
    Platform
    Replication
    Scintilla
    Serialization
    Sound
    SpatialPartition
    SpirvCross
    SpirvHeaders
    SpirvTools
    Startup
    Support
    UiWidget
    Widget
    ZLib
    LightningCore
    LightningScript
    LightningShaders
    tracy
)

target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_IMPORTS)

#plasma_copy_from_linked_libraries(PlasmaLauncher)