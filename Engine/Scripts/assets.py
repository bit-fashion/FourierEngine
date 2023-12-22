"""
@author bit-fashion

    Copyright (C) 2022 Vincent Luo All rights reserved.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, e1ither express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    Creates on 2022/12/22.
"""

from pywavefront import Wavefront

#
# .obj 模型加载
#
def obj_load(filename: str):
    # 读取 .obj 文件
    obj_file = Wavefront(filename)

    # 访问顶点数据
    vertices = obj_file.vertices
    print("vertices count:", len(vertices))
    for vertex in vertices:
        print("vertices:", vertex)

    # 访问法线数据
    normals = obj_file.normals
    print("normals count:", len(normals))
    for normal in normals:
        print(normals, normal)

    # 访问纹理坐标数据
    tex_coords = obj_file.texcoords
    print("texcoord count:", len(tex_coords))
    for tex_coord in tex_coords:
        print("texcoord:", tex_coord)

    # 访问面数据
    faces = obj_file.faces
    print("face count:", len(faces))
    for face in faces:
        print("face:", face)
