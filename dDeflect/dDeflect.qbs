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
            "src/*/*.qrc",
            "src/gui/qml/ExecutableTab.qml",
            "src/gui/qml/ObfuscationTab.qml",
            "src/gui/qml/PackingTab.qml",
            "src/gui/qml/SourceCodeTab.qml",
            "src/gui/qml/main.qml",
            "src/ApplicationManager/DMethods/*.cpp",
            "src/ApplicationManager/DMethods/*.h"

        ]

        Group {     // Properties for the produced executable
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: "bin"
        }

        Group {
            qbs.install: true
            qbs.installDir: "bin"
            files: [
                "src/core/descriptions/"
            ]
        }

        Group {
            qbs.install: true
            qbs.installDir: "bin/data"
            files: [
                "src/core/detection/",
                "src/core/handlers/",
                "src/core/helper_func/",
                "src/gui/qml/AddMethodsWindow.qml",
            ]
        }

        Depends { name: "Qt"; submodules: ["core", "widgets", "quick", "gui"] }
        cpp.warningLevel: "all"
        cpp.includePaths: ["src"]
        cpp.cxxFlags: ["-std=c++11","-Wno-unknown-pragmas","-Wno-reorder","-Wno-unused-local-typedefs"]
        cpp.defines: ["PROJECT_VERSION=\"" + version + "\""]

        Group {
            files: [
                "src/core/adding_methods/wrappers/elfaddingmethods.cpp",
                "src/core/adding_methods/wrappers/elfaddingmethods.h",
                "src/core/file_types/codedefines.cpp",
                "src/core/file_types/codedefines.h",
                "src/gui/qml/ObfuscationTab.qml",
                "src/gui/qml/PackingTab.qml",
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
            "src/ApplicationManager/applicationmanager.cpp",
            "src/ApplicationManager/applicationmanager.h",
            "src/ApplicationManager/DJsonParser/djsonparser.cpp",
            "src/ApplicationManager/DJsonParser/djsonparser.h",
            "src/ApplicationManager/dsettings.h",
            "src/ApplicationManager/dsettings.cpp",
            "src/ApplicationManager/dlogger.h",
            "src/ApplicationManager/dlogger.cpp",
            "src/ApplicationManager/DSourceCodeParser/*.cpp",
            "src/ApplicationManager/DSourceCodeParser/*.h",
            "src/DMethods/*.cpp",
            "src/DMethods/*.h",
            "src/gui/qml/DynamicRadioButtons.qml",
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
