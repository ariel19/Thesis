import qbs

CppApplication {
    property string version: "0.0.1"

    name: "dDeflect"
    type: "application" // To suppress bundle generation on Mac
    consoleApplication: true
    files: [
        "src/*/*.cpp",
        "src/*/*.h",
        "src/*/*.qrc",
    ]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
        qbs.installDir: "bin"
    }

    Depends { name: "Qt"; submodules: ["core", "widgets", "quick"] }
    cpp.warningLevel: "all"
    cpp.cxxFlags: ["-std=c++11","-Wno-unknown-pragmas","-Wno-reorder","-Wno-unused-local-typedefs"]
    cpp.defines: ["PROJECT_VERSION=\"" + version + "\""]

    Group {
        condition: qbs.targetOS == "windows"
        cpp.dynamicLibraries: []
    }

    Group {
        condition: qbs.targetOS == "linux"
        cpp.dynamicLibraries: ["boost_system"]
    }
}

