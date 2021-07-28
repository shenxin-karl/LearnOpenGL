# OpenGL Demo

这是学习 OpenGL 时, 编写的例子.

1. **blinn-phong 着色模型**
   <img src="readme_resource/blinn-phong.png" alt="blinn-phong" style="zoom:67%;" />
2. **normal mapping 切线空间的法线贴图**
   <img src="readme_resource/normal_mapping.png" alt="normal_mapping" style="zoom:67%;" />
3. **parallax mapping 视差贴图**
   <img src="readme_resource/parallax_mapping.png" alt="parallax_mapping" style="zoom:67%;" />
4. **shadow mapping 定向光阴影映射**
   <img src="readme_resource/shadow_mapping.png" alt="shadow_mapping" style="zoom: 67%;" />
5. **point shadow 点光源阴影映射**
   <img src="readme_resource/poing_shadow.png" alt="poing_shadow" style="zoom:67%;" />
6. **pbr sphere 着色**
   <img src="readme_resource/pbr_sphere.png" alt="pbr_sphere" style="zoom:67%;" />
7. **IBL 环境光**
   <img src="readme_resource/AK47.png" alt="AK47" style="zoom:67%;" />
8. **deferred shading 延迟渲染**
   <img src="readme_resource/deferred_shading.png" alt="deferred_shading" style="zoom:67%;" />
9. **SSAO(screen space ambinet occlusion) 环境光遮蔽**
   <img src="readme_resource/SSAO.png" alt="SSAO" style="zoom:67%;" />

## Build

使用 **visual studio 2019** 构建. 部分使用到了 **C++20 特性**

直接使用 **vs2019 cmake** 构建. 

生成以后会在 `out/build/x64-Debug` 下生成 `exe` 文件. **运行时可能会找不到库文件.**

需要将 `assimp/lib` 下的三个库文件拷贝到 `out/build/x64-Debug` 后可以启动

## 使用

启动程序以后, 会看到下面的界面
![image-20210723125307523](readme_resource/select_demo.png)

这时候可以选择展示的 `demo`

每个 `demo` 都可 `scene` 和 `camera` 窗口. `w, s, a, d` 可以移动. 启动 `mouse` 可以使用鼠标变换视角.  也可以通过 拖动 `pitch` 和 `yaw` 的值. 来修改观察方向
<img src="readme_resource/camera.png" alt="image-20210723125427357" style="zoom: 67%;" />

