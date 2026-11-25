_This project has been created as part of the 42 curriculum by yyamasak._

# scop

## Description

scop は、OpenGL を使って Wavefront OBJ 形式（`.obj`）の 3D モデルを表示する 42 のプロジェクトです。

**目的**

- OpenGL の描画パイプライン（シェーダー、VBO/VAO、深度バッファ）を理解する
- 行列計算（Model / View / Projection）を自前で実装する
- `.obj` パーサーを自前で実装する（行列・シェーダー・`.obj` 用の外部ライブラリは使わない）

**主な機能**

- `.obj` ファイルの読み込みと表示（三角形でない面の三角形分割に対応）
- 透視投影と隠面処理（深度バッファ／面カリング）
- 面ごとに異なる色（グレーの濃淡）で表示
- 中心軸を中心にした自動回転
- キー操作で 3 軸方向に移動
- キー操作でカラーとテクスチャを切り替え（フェードで滑らかに遷移）

## Instructions

### 必要なもの

- C++ コンパイラ（`c++` / `clang++`、C++20 対応）
- OpenGL 3.3 以上（Core Profile）
- GLFW 3（ウィンドウ／イベント管理のみ。GLAD は同梱）
- make、pkg-config

### インストール

```bash
# Ubuntu / Debian（ワンライナー）
make install
```

### ビルド

```bash
make        # ビルド
make clean  # オブジェクトファイルを削除
make fclean # 実行ファイルも削除
make re     # 最初からビルドし直す
```

### 実行

```bash
./scop <path/to/model.obj>
# 例
./scop resources/42.obj
```

### 操作方法

`Tab` で操作対象（Camera / Object）を切り替えます（現在のモードはウィンドウタイトルに表示）。

| キー              | 動作                                                      |
| ----------------- | --------------------------------------------------------- |
| `Tab`             | 操作対象の切り替え（Camera ↔ Object）                     |
| `W` / `S`         | 前後へ移動（カメラの向きに沿う。Camera / Object 共通）    |
| `A` / `D`         | 左右へ移動                                                |
| `Space` / `Shift` | 上下へ移動                                                |
| `↑` / `↓`         | （Object）X 軸まわりに回転                                |
| `←` / `→`         | （Object）Y 軸まわりに回転                                |
| `Q` / `E`         | （Object）Z 軸まわりに回転                                |
| `R`               | 自動回転の停止 / 再開                                     |
| マウス移動        | カメラの向きを変更（左クリックでロック切り替え）          |
| スクロール        | ズーム                                                    |
| `T`               | カラー ↔ テクスチャ切り替え（フェード）                   |
| `H`               | 操作一覧を標準出力に表示（起動時にも表示）                |
| `ESC`             | 終了                                                      |

不正な `.obj`（存在しない・頂点が範囲外・空など）を渡すと、ウィンドウを開く前にエラーメッセージを表示して終了コード 1 で終わります。

### テスト

```bash
./tests/run_obj_tests.sh          # パーサーのテスト（ケースは tests/cases.json）
./tests/run_obj_tests.sh 添字      # カテゴリで絞り込み
```

## Technical Choices

- **ライブラリ**: GLFW（ウィンドウ／入力）と GLAD（OpenGL ローダー）のみ。行列・シェーダー読み込み・`.obj` パースはすべて自前実装で、GLM 等は使用しない。
- **`.obj` パーサー** (`srcs/WaveFrontObjectLoader.cpp`): `v` と `f` を読み込み、`f 1/2/3` 形式や負のインデックスに対応。不正な入力（範囲外の添字、座標不足、空など）は行番号付きの例外で拒否する。
  - **三角形分割**: Fan 分割（`(v0, vi, vi+1)`）。凸多角形なら正しく分割できる（凹多角形はボーナスのため対象外）。
  - **中心と大きさ**: バウンディングボックスの中心を原点に移し、最大辺が 2 になるよう正規化（頂点の平均は密な側に偏り、回転軸がずれるため）。
  - **色**: 面（元の多角形）ごとに異なるグレー。
  - **UV**: 面の法線の主軸に垂直な平面へ射影（縦横比を保つので引き伸ばしが起きない）。
- **ログ** (`srcs/Logger.cpp`): 出力はすべて `Logger` クラス経由。`info` は標準出力、`warn` / `error` は標準エラーに出す（`> log.txt` でもエラーが見えるように分離）。
- **行列** (`srcs/Matrix.cpp`): 列優先の `ft_math::Mat4`。平行移動・X/Y/Z 軸回転・透視投影を実装。

## Resources

### 参考資料

- [LearnOpenGL – Getting started](https://learnopengl.com/Getting-started/OpenGL) — OpenGL の基礎（コンテキスト、シェーダー、バッファ）
- [opengl-tutorial.org – Tutorial 3: Matrices](https://www.opengl-tutorial.org/jp/beginners-tutorials/tutorial-3-matrices/) — MVP 行列
- [高校数学の美しい物語（manabitimes）](https://manabitimes.jp/math/983) — 回転・行列の数学的背景
- [Wavefront OBJ 形式について（gist）](https://gist.github.com/zgock999/6c74f16ae4b1958e8b8977ec3d26deb5) — `.obj` のフォーマット仕様
- [motatamota/scop](https://github.com/motatamota/scop) — 全体構成の参考

### AI の使用について

AI（Claude）を以下の用途で使用しました。

| 用途                                 | 対象                              |
| ------------------------------------ | --------------------------------- |
| OpenGL や行列計算の概念の説明        | MVP 行列、透視投影                |
| 課題要件との差分チェック・レビュー   | 実装全体                          |
| README・ドキュメントの下書き         | `README.md`                       |

> TODO(yyamasak): AI が生成したコードをそのまま使った範囲と、内容を理解・検証した方法を、事実に沿って追記すること。
