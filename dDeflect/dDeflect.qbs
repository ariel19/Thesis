import qbs

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    consoleApplication: true
    files: ["src/*/*.cpp", "src/*/*.h", "src/*/*.qrc"]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
        qbs.installDir: "bin"
    }

    Depends { name: "Qt"; submodules: ["core", "widgets", "quick"] }
}

