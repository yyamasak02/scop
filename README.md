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

- C++ コンパイラ（例: `c++` / `clang++`、C++XX 対応）
- OpenGL X.X 以上
- <ウィンドウ／イベント管理ライブラリ（例: GLFW）>
- make

### インストール

```bash
# 例: Ubuntu
sudo apt install <必要なパッケージ>
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

| キー      | 動作                        |
| --------- | --------------------------- |
| `W` / `S` | <例: Y 軸方向に移動>        |
| `A` / `D` | <例: X 軸方向に移動>        |
| `Q` / `E` | <例: Z 軸方向に移動>        |
| `T`       | カラー ↔ テクスチャ切り替え |
| `ESC`     | 終了                        |

## Technical Choices

- <採用したライブラリと理由>
- <`.obj` パーサーの設計（頂点・法線・UV・面の扱い、三角形分割のアルゴリズム）>
- <行列ライブラリの実装方針>

## Resources

### 参考資料

- [LearnOpenGL – Getting started](https://learnopengl.com/Getting-started/OpenGL) — OpenGL の基礎（コンテキスト、シェーダー、バッファ）
- [opengl-tutorial.org – Tutorial 3: Matrices](https://www.opengl-tutorial.org/jp/beginners-tutorials/tutorial-3-matrices/) — MVP 行列
- [高校数学の美しい物語（manabitimes）](https://manabitimes.jp/math/983) — <参照した数学的内容>
- [Wavefront OBJ 形式について（gist）](https://gist.github.com/zgock999/6c74f16ae4b1958e8b8977ec3d26deb5) — `.obj` のフォーマット仕様
- [motatamota/scop](https://github.com/motatamota/scop) — <参考にした点（例: 全体構成）>

### AI の使用について

AI（<ツール名、例: Claude>）を以下の用途で使用しました。

| 用途                                | 対象                       |
| ----------------------------------- | -------------------------- |
| <例: OpenGL や行列計算の概念の説明> | <例: MVP 行列、透視投影>   |
| <例: コードレビュー>                | <例: `srcs/ObjLoader.cpp`> |
| <例: README・ドキュメントの下書き>  | `README.md`                |

<AI が生成したコードをそのまま使った部分の有無、使った場合はその範囲と、内容を理解・検証した方法を書く>
