# OpenGL Texture 完全まとめ

## Textureとは？

これまでの頂点カラー方式では、

```cpp
// position      color
0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f
```

のように各頂点に色を持たせていた。

ラスタライザが色を補間することで三角形内部の色を生成していた。

---

問題は細かい模様を表現する場合。

例えばレンガ模様を表現したい場合、

```text
■■■■■■■■
■■■■■■■■
■■■■■■■■
```

大量の頂点と色データが必要になる。

結果として

- 頂点数増加
- メモリ増加
- 転送量増加

が発生する。

---

そこでTextureを使う。

Textureとは

```text
画像
```

のことである。

例えば

```text
+------------+
| レンガ画像 |
+------------+
```

を一枚用意し、

ポリゴンに貼り付けることで細かい表現を実現する。

---

# Texture Coordinate (UV座標)

Textureを貼るためには、

```text
ポリゴンのどこ
↓
画像のどこ
```

を対応付ける必要がある。

---

OpenGLでは画像上の座標を

```text
(0,1)      (1,1)
+------------+
|            |
|            |
|            |
+------------+
(0,0)      (1,0)
```

で表す。

---

例えば三角形

```text
    A
   / \
  /   \
 B-----C
```

に対して

```cpp
A -> (0.5, 1.0)
B -> (0.0, 0.0)
C -> (1.0, 0.0)
```

を指定する。

これは

```text
A → 画像上中央
B → 画像左下
C → 画像右下
```

を意味する。

---

重要なのは

```text
頂点がTextureを持つ
```

のではなく

```text
頂点がTexture上の位置(UV)を持つ
```

ことである。

---

頂点データは実際には

```cpp
struct Vertex {
    vec3 position;
    vec3 color;
    vec2 uv;
};
```

のようなイメージ。

---

# ラスタライザによるUV補間

例えば三角形中央のFragment

```text
    A
   / \
  / X \
 B-----C
```

に対して

UVも自動補間される。

```text
A=(0.5,1.0)
B=(0.0,0.0)
C=(1.0,0.0)

↓

X=(0.5,0.5)
```

のようになる。

---

# Sampling

Textureから色を取得することを

```text
Sampling
```

という。

---

Fragment Shader

```glsl
texture(myTexture, uv)
```

で取得できる。

例えば

```glsl
uv = (0.2, 0.7)
```

なら

```text
画像横20%
画像縦70%
```

の位置の色を取得する。

---

# Texture Wrapping

通常UVは

```text
0.0 ～ 1.0
```

の範囲。

---

もし

```cpp
uv = (2.3, 1.7)
```

だった場合どうするか？

---

## GL_REPEAT

デフォルト

```text
ABCABCABCABC
```

のように繰り返す。

---

## GL_MIRRORED_REPEAT

```text
ABCDEEDCBAABCDE
```

のように反転しながら繰り返す。

---

## GL_CLAMP_TO_EDGE

端で固定する。

```text
AAAAABBBBBBBBBB
```

のようになる。

---

## GL_CLAMP_TO_BORDER

範囲外を指定色にする。

```cpp
float borderColor[] = {
    1.0f,
    1.0f,
    0.0f,
    1.0f
};
```

なら黄色になる。

---

# Texture Filtering

UVは浮動小数点である。

```cpp
uv = (0.532, 0.718)
```

のような値になる。

---

しかしTextureは

```text
Pixel
Pixel
Pixel
```

しか持たない。

どの色を返すか決める必要がある。

---

## GL_NEAREST

最も近いTexelを選ぶ。

```text
□ □
□ X
```

ならXに最も近いTexelを返す。

---

特徴

```text
カクカク
ドット絵向き
```

---

## GL_LINEAR

周囲4Texelを補間する。

---

特徴

```text
なめらか
自然
```

---

設定

```cpp
glTexParameteri(
    GL_TEXTURE_2D,
    GL_TEXTURE_MAG_FILTER,
    GL_LINEAR
);
```

---

# Mipmap

遠くの物体に巨大Textureを使うのは非効率。

例えば

```text
2048x2048
```

のTextureを

```text
画面上では10px
```

しか使わない場合。

---

問題

- 無駄
- ノイズ
- チラつき

---

そこでMipMapを使う。

---

自動生成される縮小版Texture

```text
2048x2048
1024x1024
512x512
256x256
128x128
64x64
...
```

---

遠くの物体には小さいTextureを使用する。

---

生成

```cpp
glGenerateMipmap(GL_TEXTURE_2D);
```

---

# Textureを読み込む

OpenGLは

```text
jpg
png
```

を直接読めない。

---

そこで

```cpp
stb_image.h
```

を使う。

---

導入

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
```

---

読み込み

```cpp
int width;
int height;
int nrChannels;

unsigned char* data =
    stbi_load(
        "container.jpg",
        &width,
        &height,
        &nrChannels,
        0
    );
```

---

結果

```text
RGB配列
```

が得られる。

---

# Textureオブジェクト生成

```cpp
unsigned int texture;

glGenTextures(1, &texture);
```

---

Bind

```cpp
glBindTexture(
    GL_TEXTURE_2D,
    texture
);
```

---

# GPUへ転送

```cpp
glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    width,
    height,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    data
);
```

---

イメージ

```text
jpg
↓
stbi_load

CPUメモリ
↓
glTexImage2D

GPU Texture
```

---

MipMap生成

```cpp
glGenerateMipmap(GL_TEXTURE_2D);
```

---

解放

```cpp
stbi_image_free(data);
```

---

# VertexにUVを追加

```cpp
float vertices[] = {
    // pos          // color        // uv
     0.5f,0.5f,0.0f, 1,0,0,         1,1,
     0.5f,-0.5f,0.0f,0,1,0,         1,0,
    -0.5f,-0.5f,0.0f,0,0,1,         0,0,
    -0.5f,0.5f,0.0f,1,1,0,          0,1
};
```

---

Attribute登録

```cpp
glVertexAttribPointer(
    2,
    2,
    GL_FLOAT,
    GL_FALSE,
    8 * sizeof(float),
    (void*)(6 * sizeof(float))
);

glEnableVertexAttribArray(2);
```

---

# Vertex Shader

入力

```glsl
layout(location = 2)
in vec2 aTexCoord;
```

---

出力

```glsl
out vec2 TexCoord;

TexCoord = aTexCoord;
```

---

# Fragment Shader

Textureを扱うために

```glsl
uniform sampler2D ourTexture;
```

を定義する。

---

Sampling

```glsl
FragColor =
    texture(
        ourTexture,
        TexCoord
    );
```

---

流れ

```text
UV
↓
Texture参照
↓
色取得
↓
描画
```

---

# Texture Unit

複数Textureを扱う仕組み。

---

有効化

```cpp
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, texture1);

glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, texture2);
```

---

Shader

```glsl
uniform sampler2D texture1;
uniform sampler2D texture2;
```

---

対応付け

```cpp
glUniform1i(
    glGetUniformLocation(
        shader.ID,
        "texture1"
    ),
    0
);

glUniform1i(
    glGetUniformLocation(
        shader.ID,
        "texture2"
    ),
    1
);
```

---

結果

```text
texture1
↓
GL_TEXTURE0

texture2
↓
GL_TEXTURE1
```

となる。

---

# Texture合成

GLSLのmix関数を使う。

```glsl
FragColor =
    mix(
        texture(texture1, TexCoord),
        texture(texture2, TexCoord),
        0.2
    );
```

---

意味

```text
80%
texture1

20%
texture2
```

---

# 画像が上下反転する問題

OpenGLは

```text
(0,0)
↓
左下
```

を原点とする。

---

しかし画像ファイルは通常

```text
(0,0)
↓
左上
```

である。

---

対策

```cpp
stbi_set_flip_vertically_on_load(true);
```

を読み込み前に実行する。

---

# 最終まとめ

Texture描画の流れ

```text
画像ファイル

↓ stbi_load

CPU上のRGB配列

↓ glTexImage2D

GPU Texture

↓ sampler2D

Fragment Shader

↓ texture()

色取得

↓
画面描画
```

Textureは

```text
画像
```

ではなく最終的には

```text
GPUが高速参照できる
2次元データベース
```

として扱われる。
