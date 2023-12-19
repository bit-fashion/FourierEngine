/* ************************************************************************
 *
 * Copyright (C) 2022 Vincent Luo All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, e1ither express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ************************************************************************/

/* Creates on 2023/12/14. */

/*
 ===============================
   @author bit-fashion
 ===============================
*/
#ifndef _SPORTS_ENGINE_CAMERA_H_
#define _SPORTS_ENGINE_CAMERA_H_

#include <Typedef.h>
// glm
#include <Math.h>

class Camera {
public:
    // MV 矩阵
    const glm::mat4 &GetViewMatrix() const { return m_ViewMatrix; };
    const glm::mat4 &GetProjectionMatrix() const { return m_ProjectionMatrix; };

    // 更新摄像机数据，由子类实现
    virtual void UpdateCamera() = 0;

protected:
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;
};

#endif /* _SPORTS_ENGINE_CAMERA_H_ */
