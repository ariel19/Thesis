import qbs
Project {
    CppApplication {
        property string version: "0.0.1"

        name: "dDeflect"
        type: "application" // To suppress bundle generation on Mac
        consoleApplication: true
        files: [
            "src/*/*.cpp",
           "src/*/*.h",
           "src/*/*/*.cpp",
           "src/*/*/*.h",
           "src/*/*/*/*.cpp",
           "src/*/*/*/*.h",
           "src/*/*/*/*/*.cpp",
           "src/*/*/*/*/*.h",
           "src/*/*.qrc"
        ]

        Group {     // Properties for the produced executable
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: "bin"
        }

        Depends { name: "Qt"; submodules: ["core", "widgets", "quick"] }
        cpp.warningLevel: "all"
        cpp.includePaths: ["src"]
        cpp.cxxFlags: ["-std=c++11","-Wno-unknown-pragmas","-Wno-reorder","-Wno-unused-local-typedefs"]
        cpp.defines: ["PROJECT_VERSION=\"" + version + "\""]

        Group {
            files: [
            ]
            condition: qbs.targetOS == "windows"
            cpp.dynamicLibraries: []
        }

        Group {
            files: [
                "src/core/adding_methods/wrappers/elfaddingmethods.cpp",
                "src/core/adding_methods/wrappers/elfaddingmethods.h",
                "src/core/file_types/codedefines.cpp",
                "src/core/file_types/codedefines.h",
                "src/core/file_types/pehelpers.cpp",
                "src/core/file_types/pehelpers.h",
            ]
            condition: qbs.targetOS == "linux"
            cpp.dynamicLibraries: ["boost_system"]
        }
    }

    CppApplication {
        name: "tester"
        type: "application" // To suppress bundle generation on Mac
        consoleApplication: true
        files: [
           "src/core/adding_methods/wrappers/*.cpp",
           "src/core/adding_methods/wrappers/*.h",
           "tests/src/*.cpp",
           "tests/src/*.h",
           "src/core/file_types/*.cpp",
           "src/core/file_types/*.h",
        ]
        Depends { name: "Qt"; submodules: ["core"] }
        cpp.warningLevel: "all"
        cpp.cxxFlags: ["-Wold-style-cast", "-std=c++11"]
        cpp.includePaths: ["src"]

        Group {     // Properties for the produced executable
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: "bin"
        }
    }
}
