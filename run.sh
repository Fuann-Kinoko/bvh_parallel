#!/bin/bash

cmake --build build

# copy
cp build/BVHVisualization ./BVHVisualization

# select model, if no arg then the model is Cow
DEFAULT_MODEL="Cow"
MODEL="$DEFAULT_MODEL"

# select model by arg
if [ $# -ge 1 ]; then
    case "$1" in
        Dragon|Cow|Face|Car)
            MODEL="$1"
            ;;
        *)
            echo "error：invalid arg '$1'"
            echo "avaliable: Dragon, Cow, Face, Car"
            exit 1
            ;;
    esac
fi


./BVHVisualization --no_gui --no_render "$MODEL"

