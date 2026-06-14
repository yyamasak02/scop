# 内容まとめ

## 1. OpenGLの描画パイプライン

まず今やっているのはこの流れ。

```text
CPU
 ↓
Vertex Data
 ↓
Vertex Shader
 ↓
Rasterizer
 ↓
Fragment Shader
 ↓
Framebuffer
 ↓
画面
```

---

## 2. Shaderとは

Shaderは

```text
GPU上で動くプログラム
```

です。

CPUではなくGPUで実行される。

---

Vertex Shader

```text
頂点ごとの処理
```

---

Fragment Shader

```text
ピクセルごとの処理
```

---

## 3. GLSL

Shaderを書く言語。

正式名称

```text
OpenGL Shading Language
```

---

C言語っぽい。

```glsl
#version 330 core

void main()
{
}
```

---

## 4. Vertex Shader

入力

```glsl
layout(location = 0) in vec3 aPos;
```

---

出力

```glsl
out vec3 ourColor;
```

---

必ず

```glsl
gl_Position
```

を設定する。

```glsl
gl_Position = vec4(aPos, 1.0);
```

---

## 5. Fragment Shader

入力

```glsl
in vec3 ourColor;
```

---

出力

```glsl
out vec4 FragColor;
```

---

最終色を決める。

```glsl
FragColor = vec4(ourColor, 1.0);
```

---

## 6. Shader間通信

Vertex Shader

```glsl
out vec3 color;
```

---

Fragment Shader

```glsl
in vec3 color;
```

---

型と名前が一致すると

```text
自動接続
```

される。

---

## 7. Uniform

CPUからShaderへ値を渡す仕組み。

```glsl
uniform vec4 ourColor;
```

---

CPU

```cpp
glUniform4f(...)
```

---

用途

```text
色
時間
モデル行列
カメラ行列
射影行列
```

など。

---

## 8. Vertex Attribute

頂点ごとのデータ。

例

```text
位置
色
法線
UV
```

---

CPU

```cpp
glVertexAttribPointer(...)
```

---

Vertex Shader

```glsl
layout(location = 0) in vec3 aPos;
```

---

## 9. 頂点色

頂点ごとに色を持たせる。

```cpp
[x y z][r g b]
```

---

Vertex Shader

```glsl
layout(location = 1)
in vec3 aColor;
```

---

## 10. Rasterizer

超重要。

Vertex Shaderは

```text
頂点
```

しか知らない。

---

しかし画面は

```text
大量のピクセル
```

で構成される。

---

その間を埋めるのが

```text
Rasterizer
```

---

役割

```text
三角形内部のFragment生成
属性補間
```

---

## 11. Fragment Interpolation

頂点色

```text
赤
緑
青
```

しかない。

---

Rasterizerが

```text
どの頂点に近いか
```

を計算して

```text
赤 30%
緑 20%
青 50%
```

のように補間する。

---

だから

```text
グラデーション
```

になる。

---

## 12. Shaderクラス

毎回

```cpp
glCompileShader
glLinkProgram
glUseProgram
```

を書くのが面倒。

---

そこで

```cpp
Shader shader(
    "shader.vs",
    "shader.fs"
);
```

というクラスを作る。

---

責務

```text
ファイル読込
コンパイル
リンク
エラーチェック
uniform設定
```

---

# 単語まとめ

---

## Shader

GPU上で動くプログラム

---

## GLSL

OpenGL用シェーダー言語

---

## Vertex Shader

頂点単位の処理

---

## Fragment Shader

ピクセル単位の処理

---

## Vertex

頂点

```text
(x,y,z)
```

---

## Fragment

描画途中のピクセル

最終的に画面へ出力される候補。

---

## Vertex Attribute

頂点ごとのデータ

例

```text
位置
色
UV
法線
```

---

## Uniform

全頂点共通のデータ

例

```text
色
時間
行列
```

---

## in

入力

```glsl
in vec3 position;
```

---

## out

出力

```glsl
out vec3 color;
```

---

## vec2

2要素ベクトル

```glsl
vec2 uv;
```

---

## vec3

3要素ベクトル

```glsl
vec3 pos;
```

---

## vec4

4要素ベクトル

```glsl
vec4 color;
```

---

## Swizzling

ベクトル成分の並べ替え

```glsl
v.xyz
v.rgb
v.yzx
```

---

## gl_Position

Vertex Shaderの最終出力位置

```glsl
gl_Position = vec4(...);
```

---

## FragColor

Fragment Shaderの最終色

```glsl
FragColor = vec4(...);
```

---

## VBO

Vertex Buffer Object

頂点データをGPUへ送るバッファ

---

## VAO

Vertex Array Object

Vertex Attribute設定を保存するオブジェクト

---

## glVertexAttribPointer

VBO内データを

```text
どこから
どの型で
何個読むか
```

を設定する関数

---

## layout(location = n)

Vertex Attribute番号

```glsl
layout(location = 0)
```

↓

```text
属性0
```

---

## Rasterizer

三角形をFragmentへ変換するステージ

---

## Interpolation

補間

頂点データから

```text
中間値
```

を生成する処理

---

## Barycentric Coordinates

バリセントリック座標

三角形内の点を

```text
A 30%
B 20%
C 50%
```

のような重みで表現する方法。

GPUはこれを使って属性補間する。

---

## Shader Program

Vertex Shader と Fragment Shader をリンクしたもの

```cpp
glCreateProgram()
```

で作る。

---

## Shader Object

個別のシェーダー

```cpp
glCreateShader()
```

で作る。

---
