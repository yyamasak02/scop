# scop

OpenGL を使って `.obj` モデルを表示するための 42 プロジェクトです。

## ビルド

```bash
make
```

生成物:

- 実行ファイル: `./scop`
- オブジェクト: `./objs/*.o`

## 実行

```bash
./scop
```

期待される出力:

```text
Hello World
```

## よく使うコマンド

```bash
make        # ビルド
make clean  # オブジェクト削除
make fclean # 実行ファイルも削除
make re     # フルリビルド
```

## ディレクトリ構成

```text
scop/
├── includes/
├── resources/
├── srcs/
│   └── main.cpp
├── objs/
├── Makefile
└── README.md
```

## 次の実装候補

1. ウィンドウ初期化と OpenGL コンテキスト作成
2. `.obj` ローダー（頂点・法線・面）の実装
3. MVP 行列（Model / View / Projection）の導入
4. 描画ループとキーボード操作の追加
5. エラーハンドリング・入力バリデーションの整備

## 参考資料

- https://learnopengl.com/Getting-started/OpenGL
- https://www.opengl-tutorial.org/jp/beginners-tutorials/tutorial-3-matrices/
- https://manabitimes.jp/math/983
- https://github.com/motatamota/scop