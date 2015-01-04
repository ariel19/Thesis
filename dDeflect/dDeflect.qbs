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
            condition: qbs.targetOS == "linux"
            cpp.dynamicLibraries: ["boost_system"]
        }
    }

    CppApplication {
        name: "tester"
        type: "application" // To suppress bundle generation on Mac
        consoleApplication: true
        files: [
            "src/ApplicationManager/applicationmanager.cpp",
            "src/ApplicationManager/applicationmanager.h",
            "src/ApplicationManager/DJsonParser/djsonparser.cpp",
            "src/ApplicationManager/DJsonParser/djsonparser.h",
            "src/core/adding_methods/wrappers/linux/asmcodegenerator.cpp",
            "src/core/adding_methods/wrappers/linux/asmcodegenerator.h",
            "src/core/adding_methods/wrappers/linux/daddingmethods.cpp",
            "src/core/adding_methods/wrappers/linux/daddingmethods.h",
            "src/core/detection/wrappers/ddebuggerdetection.cpp",
            "src/core/detection/wrappers/ddebuggerdetection.h",
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

