import qbs 1.0
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
            "src/*/*.qrc",
            "src/gui/qml/AboutDialog.qml",
            "src/gui/qml/main.qml",
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
            ]
            condition: qbs.targetOS == "linux"

            cpp.dynamicLibraries: ["boost_system"]
            cpp.staticLibraries:["clangFrontend", "clangSerialization",
                "clangDriver","clangTooling",
                "clangParse","clangSema",
                "clangAnalysis","clangRewriteFrontend",
                "clangRewriteCore","clangEdit",
                "clangAST","clangLex","clangBasic"]
        }
    }

    CppApplication {
        name: "tester"
        type: "application" // To suppress bundle generation on Mac
        consoleApplication: true
        files: [
            "src/DSourceCodeParser/dsourcecodeparser.cpp",
            "src/DSourceCodeParser/dsourcecodeparser.h",
            "tests/src/*.cpp",
            "tests/src/*.h",
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

