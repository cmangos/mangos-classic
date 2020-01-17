
# First, get the root of the repository.
get_filename_component(ROOT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(ROOT_DIR "${ROOT_DIR}/.." ABSOLUTE)

# Set the macro directory.
list(APPEND CMAKE_MODULE_PATH "${ROOT_DIR}/cmake/macros")
