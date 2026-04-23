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

## C++ Code Review Guidelines

Act as a senior C++ engineer and review code for safety, performance, and modern standards.

### 1. Standards and Safety
- **Modern C++:** Prefer C++20/C++23 features where they improve clarity and safety (`std::span`, `std::string_view`, `auto`).
- **Memory Safety:** Flag raw pointer ownership. Recommend RAII and smart pointers (`std::unique_ptr`, `std::shared_ptr`) when ownership is needed.
- **Const Correctness:** Use `const`/`constexpr` wherever possible.

### 2. Resource Management
- **Performance:** Avoid unnecessary copies; prefer `std::move` and pass-by-`const&` where appropriate.
- **Containers:** Recommend containers based on access patterns (`std::vector`, maps, etc.).

### 3. Style and Maintainability
- **Naming:** Follow a consistent style such as Google C++ Style or LLVM Coding Standards.
- **Error Handling:** Prefer exceptions or `std::expected` (C++23) over ad-hoc error codes where practical.
- **Header Files:** Use `#pragma once`; keep includes minimal and sorted.

### 4. Feedback Format
- Label each review item as one of:
  - **[Blocking]**
  - **[Important]**
  - **[Suggestion]**
  - **[Praise]**
- Include:
  1. A short explanation of **why**.
  2. A concrete fix snippet.

#### Example
```cpp
// [Important] Prefer std::span to avoid pointer/size mismatch bugs.
void process(std::span<const int> values) {
    for (const auto value : values) {
        // ...
    }
}
```
