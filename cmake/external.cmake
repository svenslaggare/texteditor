include(ExternalProject)

ExternalProject_Add(
    glm

    URL "https://github.com/g-truc/glm/releases/download/0.9.9.0/glm-0.9.9.0.zip"

    PREFIX ${PROJECT_BINARY_DIR}/external/glm
    DOWNLOAD_DIR ${PROJECT_BINARY_DIR}/external/download/glm

    TEST_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

set(GLM_INCLUDE_DIRS ${PROJECT_BINARY_DIR}/external/glm/src/glm)
