cmake_minimum_required(VERSION 3.14)
project(split_iut CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# Obligatoire avec MSVC sous Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# nlohmann/json (en-tête unique) : seule dépendance autorisée en plus de GoogleTest
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

enable_testing()
include(GoogleTest)

add_subdirectory(benchmark)

# Chaque couche n'expose que son propre dossier : une couche ne peut inclure
# que les en-têtes des cibles auxquelles elle est liée.

# --- Domaine : logique métier pure, liée à rien ---
add_library(splitiut_domain STATIC
  src/domain/montant.cpp
  src/domain/groupe.cpp
  src/domain/depense.cpp
  src/domain/repartition.cpp
  src/domain/bilan.cpp
  src/domain/remboursement.cpp
)
target_include_directories(splitiut_domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/domain)
# Sources UTF-8 sous MSVC ("Chloé" dans le code = "Chloé" lu du JSON).
# PUBLIC : propagé à toutes les cibles qui lient le domaine.
target_compile_options(splitiut_domain PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# --- Stockage JSON : dépend du domaine et de nlohmann/json ---
add_library(splitiut_storage STATIC
  src/storage/stockage_json.cpp
)
target_include_directories(splitiut_storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/storage)
target_link_libraries(splitiut_storage
  PUBLIC splitiut_domain
  PRIVATE nlohmann_json::nlohmann_json
)

# --- Interface console ---
add_executable(splitiut
  src/cli/main.cpp
)
target_link_libraries(splitiut PRIVATE splitiut_storage splitiut_domain)

# --- Tests ---
add_executable(tests
  tests/test_main.cpp
  tests/test_montant.cpp
  tests/test_groupe.cpp
  tests/test_depense.cpp
  tests/test_repartition.cpp
  tests/test_bilan.cpp
  tests/test_remboursement.cpp
  tests/test_stockage.cpp
)
target_link_libraries(tests PRIVATE splitiut_domain splitiut_storage GTest::gtest_main)
gtest_discover_tests(tests)
