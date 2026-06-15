## この章で理解するべき内容・目的

- OpenGL には「カメラ」という機能が直接あるわけではない。
- カメラ移動は、実際には **シーン全体を逆方向に動かすことで表現する**。
- カメラの本質は、以下の情報で決まる。
  - カメラの位置
  - カメラが向いている方向
  - カメラの右方向
  - カメラの上方向

- `glm::lookAt` は、これらの情報から **ビュー行列** を作る関数である。
- WASD 入力でカメラ位置を動かす方法を理解する。
- `deltaTime` を使って、PC 性能に依存しない移動速度にする。
- マウス入力から `yaw` と `pitch` を更新し、視線方向を変える。
- スクロール入力で FOV を変え、ズームを表現する。
- 最終的には、これらをまとめて **Camera クラス** として抽象化する。

---

# Camera

前の章では、**view matrix / ビュー行列** を使ってシーン内を移動する方法を扱いました。

重要なのは、OpenGL 自体には「カメラ」という概念がないことです。

ゲームエンジン的に考えると、

> プレイヤーが前に進む
> = 実際には、世界全体を後ろに動かしているように見せる

という考え方になります。

つまり、カメラとは OpenGL の機能ではなく、**ビュー行列を使って作る仕組み**です。

この章では、3D 空間を自由に移動できる **fly style camera**、つまり空中を自由に飛び回るようなカメラを作ります。

---

# Camera / View space

## View space とは何か

**view space** とは、カメラから見た座標空間のことです。

通常、オブジェクトは world space、つまりワールド座標にあります。

例えば、

```cpp
glm::vec3 objectPos = glm::vec3(5.0f, 0.0f, -3.0f);
```

のような座標は、世界全体の中での位置です。

しかし画面に描画するときには、

> カメラから見て、そのオブジェクトがどこにあるか

に変換する必要があります。

この変換を行うのが **view matrix** です。

---

## カメラを定義するために必要なもの

カメラを作るには、次の 4 つが必要です。

```cpp
camera position
camera direction
right vector
up vector
```

日本語で言うと、

- カメラの位置
- カメラが向いている方向
- カメラから見た右方向
- カメラから見た上方向

です。

これは要するに、カメラを原点とした新しい座標軸を作るということです。

ゲームエンジン的には、

> カメラ専用の座標系を作る

と考えると理解しやすいです。

---

# 1. Camera position

カメラの位置は単純です。

```cpp
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
```

これは、カメラをワールド空間の

```cpp
(0, 0, 3)
```

に置くという意味です。

OpenGL では、基本的にカメラは最初、**-Z 方向** を向いていると考えます。

ここで注意するべきなのは、OpenGL の座標系では、

- +Z 方向：画面の手前側
- -Z 方向：画面の奥側

として扱うことが多い点です。

そのため、カメラを後ろに下げたい場合は、`+Z` 方向に移動させます。

---

# 2. Camera direction

次に、カメラがどこを向いているかを決めます。

今回は、カメラが原点 `(0, 0, 0)` を見るようにします。

```cpp
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
```

ここで少しややこしいのが、

```cpp
cameraPos - cameraTarget
```

としている点です。

普通に「カメラからターゲットへの方向」を考えるなら、

```cpp
cameraTarget - cameraPos
```

になりそうです。

しかし、ここでは view matrix 用の座標系を作るために、**カメラの正の Z 軸方向** を求めています。

OpenGL ではカメラは通常 `-Z` 方向を見るため、ここで求めている `cameraDirection` は、実際には「見ている方向」と逆向きです。

つまり名前は `cameraDirection` ですが、実態としては、

> カメラが向いている方向の逆方向

です。

ここは混乱しやすいですが、重要です。

---

# 3. Right axis

次に、カメラから見た右方向を求めます。

```cpp
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
```

ここで出てくるのが **外積 / cross product** です。

外積は、ざっくり言うと、

> 2つのベクトルに対して、両方に垂直なベクトルを作る演算

です。

ここでは、

- ワールドの上方向 `up`
- カメラの Z 軸方向 `cameraDirection`

から、カメラの右方向を作っています。

ゲームエンジン的には、かなり重要です。

例えば FPS カメラで A / D キーを押したとき、

- A：左に移動
- D：右に移動

します。

この「右方向」は、常にカメラの向きによって変わります。

だから、固定で `(1, 0, 0)` に移動するのではなく、カメラの向きから右方向を計算する必要があります。

---

# 4. Up axis

右方向と奥行き方向がわかったので、最後にカメラの上方向を求めます。

```cpp
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
```

ここでも外積を使っています。

これでカメラの座標系が完成します。

- X 軸：右方向
- Y 軸：上方向
- Z 軸：カメラ後方方向

このように、互いに垂直な 3 本の軸を作る処理は、線形代数では **Gram-Schmidt process** と呼ばれます。

ただし、最初は名前を覚えるよりも、

> カメラ用の X / Y / Z 軸を作っている

と理解できれば十分です。

---

# Look At

ここからは `LookAt` 行列の話です。

## LookAt 行列とは何か

`LookAt` 行列は、

> カメラ位置、注視点、上方向から view matrix を作る行列

です。

言い換えると、

> この位置から、この点を見るようなカメラを作ってください

という行列です。

GLM では、次のように書けます。

```cpp
glm::mat4 view;
view = glm::lookAt(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
);
```

引数は順番に、

```cpp
glm::lookAt(カメラ位置, 注視点, 上方向)
```

です。

つまり、

```cpp
glm::lookAt(position, target, up)
```

です。

---

## なぜ逆方向に動かすのか

LookAt 行列では、カメラを動かすのではなく、世界を逆方向に動かします。

例えば、カメラを右に動かしたい場合、見た目としては世界が左に動けば同じです。

つまり、

```text
カメラが右へ移動
```

と

```text
世界全体が左へ移動
```

は、画面上では同じ見え方になります。

これが view matrix の基本です。

ゲームエンジンを作るうえではかなり重要で、

> カメラは実体として世界を撮影しているのではなく、座標変換によって「そう見せている」

と理解しておくとよいです。

---

## カメラを円運動させる

次のコードでは、カメラを原点の周りで回転させています。

```cpp
const float radius = 10.0f;
float camX = sin(glfwGetTime()) * radius;
float camZ = cos(glfwGetTime()) * radius;

glm::mat4 view;
view = glm::lookAt(
    glm::vec3(camX, 0.0, camZ),
    glm::vec3(0.0, 0.0, 0.0),
    glm::vec3(0.0, 1.0, 0.0)
);
```

ここでは、

```cpp
sin(glfwGetTime())
cos(glfwGetTime())
```

を使って、時間に応じてカメラ位置を変えています。

`sin` と `cos` を使うと、円周上の座標を作れます。

つまり、

```cpp
x = sin(time) * radius;
z = cos(time) * radius;
```

によって、カメラが原点の周りをぐるぐる回るようになります。

---

# Walk around

次に、キーボード入力でカメラを移動できるようにします。

まず、カメラ用の変数を用意します。

```cpp
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
```

それぞれの意味は、

```cpp
cameraPos
```

カメラの位置。

```cpp
cameraFront
```

カメラが向いている方向。

```cpp
cameraUp
```

カメラの上方向。

です。

LookAt は次のようになります。

```cpp
view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
```

ここで大事なのは、

```cpp
cameraPos + cameraFront
```

です。

これは、

> カメラの現在位置から、カメラが向いている方向に少し進んだ点を見る

という意味です。

つまり、カメラ位置が変わっても、常に `cameraFront` の方向を見続けます。

---

## WASD 移動

キーボード入力は次のように処理します。

```cpp
void processInput(GLFWwindow *window)
{
    const float cameraSpeed = 0.05f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}
```

W / S は簡単です。

```cpp
cameraPos += cameraSpeed * cameraFront;
```

これは、カメラが向いている方向に進むという意味です。

逆に、

```cpp
cameraPos -= cameraSpeed * cameraFront;
```

は後ろに下がります。

---

## A / D の横移動

A / D は少し難しいです。

```cpp
glm::normalize(glm::cross(cameraFront, cameraUp))
```

これは、カメラの右方向を求めています。

- `cameraFront`：前方向
- `cameraUp`：上方向
- 外積：その両方に垂直な方向

なので、結果として右方向が得られます。

D キーでは右方向に進みます。

```cpp
cameraPos += right * cameraSpeed;
```

A キーでは右方向の逆、つまり左に進みます。

```cpp
cameraPos -= right * cameraSpeed;
```

これが FPS や 3D ゲームでよくある **strafe / 横移動** です。

---

## normalize が必要な理由

```cpp
glm::normalize(...)
```

を使っている理由は、移動速度を安定させるためです。

外積で得られるベクトルの長さは、状況によって変わる可能性があります。

もし正規化しないと、

- ある向きでは速く動く
- ある向きでは遅く動く

ということが起きます。

`normalize` すると、ベクトルの長さが 1 になります。

つまり、

> 方向だけを使い、長さは cameraSpeed で制御する

という形になります。

---

# Movement speed

ここでは、移動速度を PC 性能に依存させない方法を扱います。

## 固定値の cameraSpeed の問題

最初のコードでは、

```cpp
const float cameraSpeed = 0.05f;
```

のように固定値を使っていました。

しかし、これは実際のゲームでは問題があります。

なぜなら、PC によって FPS が違うからです。

例えば、

- A さんの PC：60 FPS
- B さんの PC：240 FPS

だとします。

`processInput` が毎フレーム呼ばれる場合、B さんの PC では 1 秒間に 240 回処理されます。

つまり、B さんの方が速く移動してしまいます。

これはゲームとしてよくありません。

---

## deltaTime

そこで使うのが `deltaTime` です。

```cpp
float deltaTime = 0.0f;
float lastFrame = 0.0f;
```

`deltaTime` は、

> 前のフレームから今のフレームまでにかかった時間

です。

毎フレーム、次のように計算します。

```cpp
float currentFrame = glfwGetTime();
deltaTime = currentFrame - lastFrame;
lastFrame = currentFrame;
```

そして移動速度に使います。

```cpp
float cameraSpeed = 2.5f * deltaTime;
```

これによって、

> 1 フレームあたりの移動量

ではなく、

> 1 秒あたりの移動量

として制御できます。

---

## ゲームエンジン的に重要な考え方

ゲーム開発では、ほぼすべての移動処理に `deltaTime` が関わります。

例えば、

```cpp
position += velocity * deltaTime;
```

という形です。

これは、

> フレーム数ではなく、時間に基づいて動かす

という考え方です。

ゲームエンジンを作るなら、この考え方はかなり重要です。

---

# Look around

次は、マウスで視点を動かす処理です。

キーボードだけだと前後左右には動けますが、向きを変えられません。

そのため、マウスの移動量を使って `cameraFront` を更新します。

ここで必要になるのが、

- yaw
- pitch

です。

---

# Euler angles

## Euler angles とは

Euler angles は、3D 回転を表す方法の一つです。

代表的な角度は 3 つあります。

- pitch
- yaw
- roll

それぞれの意味は、

```text
pitch：上下を見る角度
yaw：左右を見る角度
roll：首を傾けるような回転
```

です。

FPS カメラでは基本的に、

- yaw
- pitch

だけを使います。

`roll` は、飛行機や宇宙船のようなカメラで使うことが多いです。

---

## yaw と pitch から方向ベクトルを作る

マウスで視点を動かすには、

```cpp
yaw
pitch
```

を更新し、それをもとに `cameraFront` を計算します。

最終的な式はこれです。

```cpp
direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
direction.y = sin(glm::radians(pitch));
direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
```

この式は難しく見えますが、やっていることは、

> 左右の角度と上下の角度から、3D の向きベクトルを作る

ということです。

---

## yaw の初期値が -90 度の理由

デフォルトでは、次のようにします。

```cpp
yaw = -90.0f;
```

なぜかというと、三角関数の都合上、`yaw = 0` のとき、方向ベクトルは `+X` 方向を向いてしまうからです。

しかし OpenGL のカメラは、基本的に最初 `-Z` 方向を向いてほしいです。

そのため、最初から `-90` 度回転させておきます。

これにより、初期状態でカメラが `-Z` 方向を見るようになります。

---

# Mouse input

マウス入力を扱うために、まずカーソルを非表示にして、ウィンドウ内に固定します。

```cpp
glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
```

これは FPS カメラではよく使います。

マウスカーソルが画面外に出てしまうと、視点操作がしにくいからです。

---

## マウスコールバック

GLFW では、マウスが動いたときに呼ばれる関数を登録します。

```cpp
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
```

登録は次のように行います。

```cpp
glfwSetCursorPosCallback(window, mouse_callback);
```

---

## マウス入力処理の流れ

マウスで視点を変える処理は、次の流れです。

1. 前フレームからどれだけマウスが動いたかを計算する
2. その移動量を yaw / pitch に足す
3. pitch が大きくなりすぎないよう制限する
4. yaw / pitch から cameraFront を再計算する

---

## マウスの移動量を計算する

最初に、前回のマウス位置を保存しておきます。

```cpp
float lastX = 400, lastY = 300;
```

画面サイズが 800 × 600 なので、中央の位置です。

マウスが動いたら、現在位置との差分を取ります。

```cpp
float xoffset = xpos - lastX;
float yoffset = lastY - ypos;
lastX = xpos;
lastY = ypos;
```

ここで、

```cpp
float yoffset = lastY - ypos;
```

となっている点に注意です。

画面座標では、Y 座標の向きが数学的な座標と違うため、反転させています。

---

## sensitivity

マウスの感度を調整します。

```cpp
const float sensitivity = 0.1f;
xoffset *= sensitivity;
yoffset *= sensitivity;
```

これがないと、少しマウスを動かしただけで視点が大きく動きすぎます。

ゲームでいう「マウス感度」です。

---

## yaw / pitch を更新する

```cpp
yaw   += xoffset;
pitch += yoffset;
```

横方向のマウス移動は `yaw` に反映します。

縦方向のマウス移動は `pitch` に反映します。

---

## pitch を制限する

```cpp
if(pitch > 89.0f)
    pitch = 89.0f;

if(pitch < -89.0f)
    pitch = -89.0f;
```

`pitch` を `90` 度まで許すと問題が起きます。

カメラが真上や真下を向いたとき、`up` ベクトルとの関係が崩れて、画面が反転するような挙動になることがあります。

これを **LookAt flip** と呼んでいます。

そのため、`pitch` は `-89` 度から `89` 度に制限します。

一方、`yaw` は制限しません。

左右には何周でも回れてよいからです。

---

## direction を計算する

最後に、yaw / pitch から新しい方向ベクトルを作ります。

```cpp
glm::vec3 direction;
direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
direction.y = sin(glm::radians(pitch));
direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
cameraFront = glm::normalize(direction);
```

ここで作った `cameraFront` が、次の `lookAt` に使われます。

```cpp
view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
```

つまり、マウス入力によって `cameraFront` が変わり、その結果としてカメラの向きが変わります。

---

## firstMouse

最初にマウスがウィンドウに入ったとき、視点が急に飛ぶことがあります。

原因は、初回の `xpos` / `ypos` が `lastX` / `lastY` から大きく離れている場合があるからです。

その対策として、初回だけ現在のマウス位置をそのまま保存します。

```cpp
if (firstMouse)
{
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
}
```

これにより、初回の大きな差分が発生しなくなります。

---

## 最終的な mouse_callback

```cpp
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;

    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(direction);
}
```

この処理によって、FPS のように自由に視点を動かせるようになります。

---

# Zoom

最後にズームです。

ズームは、カメラを前に動かしているのではありません。

ここでは **FOV / Field of View** を変えることでズームを表現します。

FOV は視野角です。

- FOV が大きい：広く見える
- FOV が小さい：狭く見える、ズームしているように見える

---

## スクロール入力

スクロール用のコールバックを作ります。

```cpp
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;

    if (fov < 1.0f)
        fov = 1.0f;

    if (fov > 45.0f)
        fov = 45.0f;
}
```

`yoffset` は縦方向のスクロール量です。

スクロールによって `fov` を変化させます。

ただし、極端な値になると見た目がおかしくなるので、

```cpp
1.0f <= fov <= 45.0f
```

に制限します。

---

## projection matrix に反映する

FOV を変更したら、透視投影行列にも反映します。

```cpp
projection = glm::perspective(
    glm::radians(fov),
    800.0f / 600.0f,
    0.1f,
    100.0f
);
```

ここで重要なのは、`fov` は projection matrix 側の値だということです。

つまり、

- view matrix：カメラの位置・向き
- projection matrix：見え方、視野角、遠近感

を担当しています。

ゲームエンジン的には、この分離はかなり大事です。

---

## スクロールコールバックを登録する

```cpp
glfwSetScrollCallback(window, scroll_callback);
```

これでマウスホイールによるズームが可能になります。

---

# Camera class

この章で作ったカメラ処理は、コード量が多くなります。

毎回、

- `cameraPos`
- `cameraFront`
- `cameraUp`
- `yaw`
- `pitch`
- `fov`
- キーボード処理
- マウス処理
- スクロール処理
- view matrix 生成

をそのまま書くのは大変です。

そこで、これらをまとめて **Camera クラス** にします。

ゲームエンジン開発では、この抽象化が非常に重要です。

例えば最終的には、

```cpp
Camera camera;
view = camera.GetViewMatrix();
```

のように使える形にしたいです。

---

## Camera クラスに入る責務

Camera クラスには、だいたい次のような責務が入ります。

- カメラ位置を持つ
- カメラの向きを持つ
- `yaw` / `pitch` を持つ
- キーボード入力に応じて移動する
- マウス入力に応じて向きを変える
- スクロール入力に応じて FOV を変える
- view matrix を返す

つまり、Camera クラスは、

> カメラの状態と、その状態を更新する処理をまとめたもの

です。

---

# 注意点

この章で作るカメラは **fly camera** です。

つまり、空中を自由に移動できるカメラです。

ただし、ゲームによって必要なカメラは違います。

例えば、

## FPS カメラ

FPS では、基本的に地面の上を歩くので、Y 軸方向には自由に動かないことが多いです。

また、上下を見ることはできても、身体自体が上下に飛ぶわけではありません。

## Flight camera

飛行機や宇宙船のカメラでは、`roll` も重要になります。

しかし今回の実装では、`up` ベクトルを固定で

```cpp
glm::vec3(0.0f, 1.0f, 0.0f)
```

としているため、roll を含むようなカメラにはそのままでは向きません。

---

# この章の本質

この章の本質は、単に `glm::lookAt` の使い方を覚えることではありません。

重要なのは、

> 3D 空間におけるカメラとは、座標変換によって実現される仕組みである

という理解です。

ゲームエンジン開発においては、カメラは非常に重要なコンポーネントです。

なぜなら、プレイヤーが世界をどう見るかは、ほぼカメラによって決まるからです。

---

# 最低限押さえるべきコードの関係

最終的には、この流れを理解できればよいです。

```cpp
// 1. 入力によってカメラ状態を更新する
processInput(window);

// 2. マウス入力で cameraFront を更新する
mouse_callback(...);

// 3. カメラ状態から view matrix を作る
view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

// 4. FOV から projection matrix を作る
projection = glm::perspective(glm::radians(fov), aspect, near, far);

// 5. シェーダーに view / projection を渡す
shader.setMat4("view", view);
shader.setMat4("projection", projection);
```

この章は、今後ゲームエンジンで必要になる、

- カメラ制御
- 座標空間
- 入力処理
- 時間ベースの移動
- ビュー行列
- 投影行列

の基礎になります。
