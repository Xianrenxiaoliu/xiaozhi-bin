#!/usr/bin/env python3
"""
把 my-s3-28tft 板型注册进 xiaozhi-esp32 的构建系统。

用法: python3 patch_repo.py <xiaozhi-esp32 目录>

做的事：
  1. main/Kconfig.projbuild 的 choice BOARD_TYPE 里加一个 config 项
  2. main/CMakeLists.txt 的板型 if/elseif 链里加一个分支

可重复执行；已经打过补丁会自动跳过。
"""
import sys
import pathlib

BOARD_SYMBOL = "BOARD_TYPE_MY_S3_28TFT"
BOARD_DIR = "my-s3-28tft"

KCONFIG_BLOCK = """    config BOARD_TYPE_MY_S3_28TFT
        bool "My ESP32-S3 2.8 inch TFT (ILI9341 + XPT2046)"
        depends on IDF_TARGET_ESP32S3

"""

CMAKE_BLOCK = """elseif(CONFIG_BOARD_TYPE_MY_S3_28TFT)
    set(BOARD_DIR "my-s3-28tft")
    set(BUILTIN_TEXT_FONT font_noto_sans_basic_20_4)
    set(BUILTIN_ICON_FONT font_material_symbols_20_4)
"""


def patch_kconfig(path: pathlib.Path) -> str:
    text = path.read_text(encoding="utf-8")
    if BOARD_SYMBOL in text:
        return "skip (already present)"
    anchor = "    config BOARD_TYPE_BREAD_COMPACT_WIFI_LCD"
    if anchor in text:
        text = text.replace(anchor, KCONFIG_BLOCK + anchor, 1)
    else:
        start = text.index("choice BOARD_TYPE")
        end = text.index("endchoice", start)
        text = text[:end] + KCONFIG_BLOCK + text[end:]
    path.write_text(text, encoding="utf-8")
    return "patched"


def patch_cmake(path: pathlib.Path) -> str:
    text = path.read_text(encoding="utf-8")
    if BOARD_SYMBOL in text:
        return "skip (already present)"

    inserted = False
    for anchor in (
        "elseif(CONFIG_BOARD_TYPE_BREAD_COMPACT_WIFI_LCD)",
        "elseif(CONFIG_BOARD_TYPE_BREAD_COMPACT_WIFI)",
    ):
        if anchor in text:
            text = text.replace(anchor, CMAKE_BLOCK + anchor, 1)
            inserted = True
            break

    if not inserted:
        marker = "if(NOT BOARD_DIR AND NOT CMAKE_BUILD_EARLY_EXPANSION)"
        if marker not in text:
            raise SystemExit("main/CMakeLists.txt 结构变了，找不到插入点")
        pos = text.index(marker)
        end = text.rindex("endif()", 0, pos)
        text = text[:end] + CMAKE_BLOCK + text[end:]

    path.write_text(text, encoding="utf-8")
    return "patched"


def main() -> None:
    if len(sys.argv) < 2:
        raise SystemExit("用法: python3 patch_repo.py <repo-dir>")
    repo = pathlib.Path(sys.argv[1]).resolve()
    if not (repo / "main/CMakeLists.txt").is_file():
        raise SystemExit(f"{repo} 不是 xiaozhi-esp32 仓库")

    print("Kconfig :", patch_kconfig(repo / "main/Kconfig.projbuild"))
    print("CMake   :", patch_cmake(repo / "main/CMakeLists.txt"))

    board = repo / "main/boards" / BOARD_DIR
    if not board.is_dir():
        raise SystemExit(f"板级目录不存在: {board}")
    print("board   :", board)


if __name__ == "__main__":
    main()
