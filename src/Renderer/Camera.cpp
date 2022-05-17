#include "Camera.h"



void Camera::update(float dt, const Input::MouseState& mousestate, bool update_flag)
{
	float deltax = (mousestate.x_ - m_last_mouse_state.x_) * 0.002f;
	float deltay = (mousestate.y_ - m_last_mouse_state.y_) * 0.002f;
	float deltaz = (mousestate.z_ - m_last_mouse_state.z_) * 0.1;
	m_last_mouse_state = mousestate;


	if (update_flag)
	{
		if (Input::IsMouseButton(Input::MouseButton::LEFT))
		{
			m_delta_rotation[0] -= deltax;
			m_delta_rotation[1] -= deltay;
		}

		if (Input::IsMouseButton(Input::MouseButton::MIDDLE))
		{
			m_delta_move[0] += deltax;
			m_delta_move[1] += deltay;
		}


		KeyMove();

		if (Input::IsMouseButton(Input::MouseButton::RIGHT))m_delta_dist += deltay;
		m_delta_dist += deltaz;
	}


	m_time_step = dt;
	//avoid fps<10
	float percent = std::min(dt * 8.0f, 1.0f);


	mouseRotate(percent);
	mouseMiddle(percent);
	mouseRightAndScroll(percent);



	updateView();


}

void Camera::KeyMove()
{

	if (Input::IsKeyPress(Input::Key::A))
	{
		m_delta_move[0] += m_time_step * 5.0f;
	}

	if (Input::IsKeyPress(Input::Key::D))
	{
		m_delta_move[0] -= m_time_step * 5.0f;
	}
	if (Input::IsKeyPress(Input::Key::W))
	{

		m_delta_move[1] += m_time_step * 5.0f;
	}
	if (Input::IsKeyPress(Input::Key::S))
	{

		m_delta_move[1] -= m_time_step * 5.0f;
	}
}


Vec3f Camera::get_up_direction() const
{
	return TinyMath::ToYAxis(m_rotation);
}

Vec3f Camera::get_right_direction() const
{
	return TinyMath::ToXAxis(m_rotation);
}

Vec3f Camera::get_forward_direction() const
{
	return -TinyMath::ToZAxis(m_rotation);
}


void Camera::mouseRotate(float percent)
{

	if (TinyMath::IsEqual(m_delta_rotation[0], 0.0f, FLT_EPSILON) && TinyMath::IsEqual(m_delta_rotation[1], 0.0f, FLT_EPSILON)) return;

	float deltax = m_delta_rotation[0] * percent;
	float deltay = m_delta_rotation[1] * percent;
	m_delta_rotation[0] -= deltax;
	m_delta_rotation[1] -= deltay;
	auto right_dir = get_right_direction();
	auto up_dir = get_up_direction();

	Quaternion rotate_y(right_dir, deltay);
	Quaternion rotate_x(up_dir, deltax);

	m_rotation = rotate_x*m_rotation;
	m_rotation = rotate_y * m_rotation;
	m_rotation = TinyMath::Normalize(m_rotation);
	updatePostion();
}


void Camera::mouseMiddle(float percent)
{

	if (TinyMath::IsEqual(m_delta_move[0], 0.0f, FLT_EPSILON) && TinyMath::IsEqual(m_delta_move[1], 0.0f, FLT_EPSILON)) return;

	float deltax = m_delta_move[0] * percent;
	float deltay = m_delta_move[1] * percent;
	m_delta_move[0] -= deltax;
	m_delta_move[1] -= deltay;
	m_focal_point -= get_right_direction() * deltax * m_xspeed * m_distance * 7.345f;
	m_focal_point += get_up_direction() * deltay * m_yseppd * m_distance * 5.444f;
	updatePostion();
}
void Camera::mouseRightAndScroll(float percent)
{
	if (TinyMath::IsEqual(m_delta_dist, 0.0f, FLT_EPSILON)) return;
	float delta = m_delta_dist * percent;
	m_delta_dist -= delta;


	auto speed = scrollSpeed();
	float new_dist = m_distance - delta * speed;
	if (new_dist > m_near && new_dist < m_far) m_distance = new_dist;
	updatePostion();
}
float Camera::scrollSpeed()
{
	float distance = m_distance * 0.3f;
	distance = std::max(distance, 0.0f);
	float speed = distance * distance;
	speed = std::min(speed, 100.0f); // max speed = 100
	return speed;
}
void Camera::updateView()
{
	Vec3f pos = get_position();
	m_view_mat = TinyMath::LookAt(pos, m_focal_point, get_up_direction());
}

void Camera::updateProject()
{
	m_projection_mat = TinyMath::Perspective(m_fov, float(m_width) / float(m_height), m_near, m_far);
}

void Camera::updateSpeed()
{
	float x = std::min(m_width / 1000.0f, 2.4f); // max = 2.4f
	m_xspeed = 0.0487f * (x * x) - 0.2578f * x + 0.3021f;

	float y = std::min(m_height / 1000.0f, 2.4f); // max = 2.4f
	m_yseppd = 0.0487f * (y * y) - 0.2578f * y + 0.3021f;
}



