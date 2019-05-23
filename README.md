# Description
This is a modified version of [the opencv_annotation tool](https://github.com/opencv/opencv/tree/master/apps/annotation). It differs from the original version in that:
- it supports multi-class labelling (up to 10 classes, because it uses keyboard 0-9 for labelling)
- it supports YOLO annotations

# Build
```bash
cd <project-root>
mkdir build
cd build
cmake -Wno-dev ..
make
```
This will build a program called `yolo_annotation`

# Basic Usage
```bash
# See usage
./yolo_annotation -h
# make annotations
./yolo_annotation -i=<absolute-path-to-your-images-directory> 
# make yolo annotations
./yolo_annotation -i=<absolute-path-to-your-images-directory> --yolo
```