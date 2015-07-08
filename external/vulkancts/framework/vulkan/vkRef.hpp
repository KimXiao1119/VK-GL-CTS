#ifndef _VKREF_HPP
#define _VKREF_HPP
/*-------------------------------------------------------------------------
 * Vulkan CTS Framework
 * --------------------
 *
 * Copyright (c) 2015 Google Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be
 * included in all copies or substantial portions of the Materials.
 *
 * The Materials are Confidential Information as defined by the
 * Khronos Membership Agreement until designated non-confidential by
 * Khronos, at which point this condition clause shall be removed.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *//*!
 * \file
 * \brief Vulkan object reference holder.
 *//*--------------------------------------------------------------------*/

#include "vkDefs.hpp"
#include "vkStrUtil.hpp"
#include "deMeta.hpp"

namespace vk
{

namespace refdetails
{

using de::meta::EnableIf;

template<typename T>
struct RefTraits
{
	typedef DeviceInterface		Interface;
	enum { REQUIRES_DEVICE = 1 };
};

template<>
struct RefTraits<VkInstanceT>
{
	typedef PlatformInterface	Interface;
	enum { REQUIRES_DEVICE = 0 };
};

template<>
struct RefTraits<VkDeviceT>
{
	typedef DeviceInterface		Interface;
	enum { REQUIRES_DEVICE = 0 };
};

template<typename T>
struct Checked
{
	explicit inline				Checked		(typename Traits<T>::Type object_) : object(object_) {}

	typename Traits<T>::Type	object;
};

//! Check that object is not null
template<typename T>
inline Checked<T> check (typename Traits<T>::Type object)
{
	if (!object)
		throw tcu::TestError("Object check() failed", (std::string(getTypeName<T>()) + " = 0").c_str(), __FILE__, __LINE__);
	return Checked<T>(object);
}

//! Declare object as checked earlier
template<typename T>
inline Checked<T> notNull (typename Traits<T>::Type object)
{
	if (!object)
		throw tcu::InternalError("Null object was given to notNull()", (std::string(getTypeName<T>()) + " = 0").c_str(), __FILE__, __LINE__);
	return Checked<T>(object);
}

//! Allow null object
template<typename T>
inline Checked<T> allowNull (typename Traits<T>::Type object)
{
	return Checked<T>(object);
}

// \todo [2015-05-12 pyry] Can we further simplify this using RefTraits<T>::REQUIRES_DEVICE?

template<typename T>
struct RefData
{
											RefData		(const typename RefTraits<T>::Interface*	vk_,
														 VkDevice									device_,
														 typename Traits<T>::Type					object_)
															: vk	(vk_)
															, device(device_)
															, object(object_)
											{}
											RefData		(const typename RefTraits<T>::Interface*	vk_)
															: vk	(vk_)
															, device(0)
															, object(0)
											{}

	const typename RefTraits<T>::Interface*	vk;
	VkDevice								device;
	typename Traits<T>::Type				object;
};

template<>
struct RefData<VkInstanceT>
{
												RefData		(const RefTraits<VkInstanceT>::Interface*	vk_,
															 VkInstance									object_)
																: vk		(vk_)
																, object	(object_)
												{}
												RefData		(const RefTraits<VkInstanceT>::Interface*	vk_)
																: vk	(vk_)
																, object(0)
												{}

	const RefTraits<VkInstanceT>::Interface*	vk;
	Traits<VkInstanceT>::Type					object;
};

template<>
struct RefData<VkDeviceT>
{
											RefData		(const RefTraits<VkDeviceT>::Interface*	vk_,
														 VkDevice								object_)
															: vk		(vk_)
															, object	(object_)
											{}
											RefData		(const RefTraits<VkDevice>::Interface*	vk_)
															: vk	(vk_)
															, object(0)
											{}

	const RefTraits<VkDeviceT>::Interface*	vk;
	Traits<VkDeviceT>::Type					object;
};

template<typename T>
bool hasObject (const RefData<T>& ref)
{
	return ref.object != 0;
}

template<typename T>
void clearObject (typename EnableIf<RefData<T>, RefTraits<T>::REQUIRES_DEVICE == 1>::Type& ref)
{
	ref.object	= 0;
	ref.device	= 0;
}

template<typename T>
void clearObject (typename EnableIf<RefData<T>, RefTraits<T>::REQUIRES_DEVICE == 0>::Type& ref)
{
	ref.object	= 0;
}

template<typename T>
void assignObject (typename EnableIf<RefData<T>, RefTraits<T>::REQUIRES_DEVICE == 1>::Type& dst, const RefData<T>& src)
{
	dst.object	= src.object;
	dst.device	= src.device;
}

template<typename T>
void assignObject (typename EnableIf<RefData<T>, RefTraits<T>::REQUIRES_DEVICE == 0>::Type& dst, const RefData<T>& src)
{
	dst.object	= src.object;
}

template<typename T>
void destroyObject (const RefData<T>& ref)
{
	ref.vk->destroyObject(ref.device, getObjectType<T>(), ref.object);
}

template<>
inline void destroyObject<VkInstanceT> (const RefData<VkInstanceT>& ref)
{
	ref.vk->destroyInstance(ref.object);
}

template<>
inline void destroyObject<VkDeviceT> (const RefData<VkDeviceT>& ref)
{
	ref.vk->destroyDevice(ref.object);
}

template<>
inline void destroyObject<VkDeviceMemoryT> (const RefData<VkDeviceMemoryT>& ref)
{
	ref.vk->freeMemory(ref.device, ref.object);
}

template<typename T>
class RefBase
{
public:
											~RefBase	(void);

	inline const typename Traits<T>::Type&	get			(void) const throw() { return m_data.object;	}
	inline const typename Traits<T>::Type&	operator*	(void) const throw() { return get();			}
	inline operator							bool		(void) const throw() { return !!get();			}
protected:
											RefBase		(RefData<T> data) : m_data(data)	{}

	void									reset		(void);				//!< Release previous object, set to null.
	RefData<T>								disown		(void) throw();		//!< Disown and return object (ownership transferred to caller).
	void									assign		(RefData<T> data);	//!< Set new pointer, release previous pointer.

private:
	RefData<T>								m_data;
};

/*--------------------------------------------------------------------*//*!
 * \brief Movable Vulkan object reference.
 *
 * Similar to de::MovePtr.
 *//*--------------------------------------------------------------------*/
template<typename T>
class Move : public RefBase<T>
{
public:
	template<typename U>
				Move		(typename EnableIf<const typename RefTraits<U>::Interface&, RefTraits<U>::REQUIRES_DEVICE==1>::Type	vk,
							 VkDevice																							device,
							 Checked<U>																							object)
								: RefBase<T>(RefData<T>(&vk, device, object.object))
				{}

	template<typename U>
				Move		(typename EnableIf<const typename RefTraits<U>::Interface&, RefTraits<U>::REQUIRES_DEVICE==0>::Type	vk,
							 Checked<U>																							object)
								: RefBase<T>(RefData<T>(&vk, object.object))
				{}

				Move		(const typename RefTraits<T>::Interface& vk)	: RefBase<T>(RefData<T>(&vk))	{}
				Move		(RefData<T> data)								: RefBase<T>(data)				{}

	Move<T>&	operator=	(Move<T>& other);
	Move<T>&	operator=	(RefData<T> data);

	operator	RefData<T>	(void) { return this->disown(); }
};

/*--------------------------------------------------------------------*//*!
 * \brief Unique Vulkan object reference.
 *
 * Similar to de::UniquePtr.
 *//*--------------------------------------------------------------------*/
template<typename T>
class Unique : public RefBase<T>
{
public:
	template<typename U>
				Unique		(typename EnableIf<const typename RefTraits<U>::Interface&, RefTraits<U>::REQUIRES_DEVICE==1>::Type	vk,
							 VkDevice																							device,
							 Checked<U>																							object)
								: RefBase<T>(RefData<T>(&vk, device, object.object))
				{}

	template<typename U>
				Unique		(typename EnableIf<const typename RefTraits<U>::Interface&, RefTraits<U>::REQUIRES_DEVICE==0>::Type	vk,
							 Checked<U>																							object)
								: RefBase<T>(RefData<T>(&vk, object.object))
							{}

				Unique		(RefData<T> data)	: RefBase<T>(data)	{}
};

// RefBase

template<typename T>
inline RefBase<T>::~RefBase (void)
{
	this->reset();
}

template<typename T>
inline void RefBase<T>::reset (void)
{
	if (hasObject(m_data))
		destroyObject<T>(m_data);

	clearObject<T>(m_data);
}

template<typename T>
inline RefData<T> RefBase<T>::disown (void) throw()
{
	const RefData<T>	old	= m_data;
	clearObject<T>(m_data);
	return old;
}

template<typename T>
inline void RefBase<T>::assign (RefData<T> data)
{
	this->reset();
	assignObject<T>(m_data, data);
}

// Move

template<typename T>
inline Move<T>& Move<T>::operator= (Move<T>& other)
{
	if (this != &other)
		this->assign(other.disown());

	return *this;
}

template<typename T>
inline Move<T>& Move<T>::operator= (RefData<T> data)
{
	this->assign(data);
	return *this;
}

} // refdetails

using refdetails::Move;
using refdetails::Unique;
using refdetails::check;
using refdetails::notNull;
using refdetails::allowNull;

#include "vkRefUtil.inl"

} // vk

#endif // _VKREF_HPP