#pragma once
#include"Math.h"
#include"Common/Event.h"

#include<stdint.h>
using namespace TinyMath;
class Camera
{
public:
    Camera() = delete;
    Camera(float fov, uint32_t width, uint32_t height, float near, float far)
        :m_fov(fov), m_width(width), m_height(height), m_near(near), m_far(far), m_rotation(Quaternion::GetIdentity()), m_position(0.0f, 0.0f, 0.0f)
    {
        updateSpeed();
        updatePostion();
        updateView();
        updateProject();

    }



public:
    void update(float dt, const Input::MouseState& mousestate, bool update_flag = true);
    void KeyMove();

public:
    void set_distance(float distance) {
        m_distance = distance;  updatePostion(); updateView();
    }
    void set_focal_point(const Vec3f& fp) { m_focal_point = fp; updatePostion(); updateView(); }
    void set_fov(float fov) { m_fov = fov, updateProject(); }
    void set_width_and_height(uint32_t width, uint32_t height) { m_width = width; m_height = height; updateProject(); updateSpeed(); }



    float get_distance() const { return m_distance; }
    const Vec3f& get_focal_point()const { return m_focal_point; }
    const Mat4f get_view_mat()const { return  m_view_mat; }
    const Mat4f get_projection_mat()const { return m_projection_mat; }


    Vec3f get_up_direction()const;
    Vec3f get_right_direction()const;
    Vec3f get_forward_direction()const;
    const Vec3f& get_position()const { return m_position; }




private:
    void mouseRotate(float percent);
    void mouseMiddle(float percent);
    void mouseRightAndScroll(float percent);

    float scrollSpeed();
    void updatePostion() { m_position = m_focal_point - (get_forward_direction() * m_distance); };
    void updateView();
    void updateProject();
    void updateSpeed();
private:

    float		m_fov;
    float		m_near;
    float		m_far;
    float		m_distance = 0.1f;
    uint32_t	m_width;
    uint32_t	m_height;

    Vec3f       m_focal_point = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f       m_position;
    //for right hand
    Mat4f		m_view_mat;
    Mat4f		m_projection_mat;


    float		m_time_step = 0.0f;
    Quaternion	m_rotation;
    Input::MouseState	m_last_mouse_state;

    //only for MouseMiddle;
    float m_xspeed;
    float m_yseppd;


    float		m_delta_rotation[2] = { 0.0f,0.0f };		//for mouseRotate
    float		m_delta_move[2] = { 0.0f,0.0f };			//for mouseMiddle
    float		m_delta_dist = 0.0f;			//for mouseLeft and  mouseScroll
};