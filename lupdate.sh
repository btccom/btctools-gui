#/bin/sh
cd "$(dirname "$0")"
lupdate $(find . -name '*.h' -or -name '*.cpp' -or -name '*.ui') -ts ./res/locale/zh_CN.ts
