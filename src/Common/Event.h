#pragma once
#include"KeyCode.h"

#include<stdint.h>
#include<functional>

enum EventType
{
	NONE=0,
	WindowResize,
	MouseMove,
	MouseScroll,
	Key

};

struct Event
{
public:
	Event(EventType type=NONE):type_(type){}
	static EventType Type() { return NONE; }
	const EventType& GetType()const  { return type_; }
	EventType type_;
};




struct MouseMoveEvent:public Event 
{
public:
	MouseMoveEvent(float x,float y):x_(x),y_(y),Event(EventType::MouseMove){}
	static EventType Type() { return MouseMove; }

public:
	float x_;
	float y_;
};



struct MouseScrollEvent :public Event
{
public:
	MouseScrollEvent(float z):z_(z),Event(EventType::MouseScroll){}
	static EventType Type() { return MouseScroll; }

public:
	float z_;
};


struct WindowResizeEvent:public Event
{
public:
	WindowResizeEvent(uint32_t width,uint32_t height):width_(width),height_(height),Event(EventType::WindowResize){}

public:
	uint32_t width_;
	uint32_t height_;
};
//TODO KeyEvent




class EventDispatcher
{
public:
	EventDispatcher(const Event* e) :
		m_event(e) {}

	template<typename T>
	using EventFn = std::function<void(const T* e)>;

	template<typename T>
	void dispatch(const EventFn<T>& eventFn)
	{
		if (m_event->GetType() == T::Type())
		{
			eventFn((const T*)m_event);
		}
	}
private:
	const Event* m_event;
};




struct GLFWwindow;
namespace Input
{
	struct MouseState
	{
	public:
		MouseState(float x=0.0f,float y=0.0f,float z=0.0f):x_(x),y_(y),z_(z){}
		float x_, y_, z_;
	};
	using EventCallBackFunc = std::function<void(const Event*)>;
	void Init(GLFWwindow* window,const EventCallBackFunc& func);	
	bool IsKeyPress(Key keycode);
	bool IsMouseButton(MouseButton code);
}

