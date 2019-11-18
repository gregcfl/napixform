{
    "targets": [{
        "target_name": "jtransformer",
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "std=c++11" ],
        "sources": [
            "cppsrc/main.cpp"
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")"
        ],
        'libraries': [],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }]
}
