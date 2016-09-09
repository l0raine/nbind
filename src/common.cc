// This file is part of nbind, copyright (C) 2014-2016 BusFaster Ltd.
// Released under the MIT license, see LICENSE.

#include "nbind/nbind.h"

using namespace nbind;

const char *nbind :: emptyGetter = "";
const char *nbind :: emptySetter = "";

// Linkage for module-wide error message.
char *Status :: message;

void NBind :: bind_value(const char *name, cbFunction &func) {
	for(auto *bindClass : getClassList()) {
		if(!bindClass) continue;

		if(strcmp(bindClass->getName(), name) == 0) {
			bindClass->setValueConstructorJS(func);
			break;
		}
	}
}

// Type description helpers.

template <typename ArgType>
struct isSignless {
	static constexpr bool value = false;
};

template <>
struct isSignless<char> {
	static constexpr bool value = true;
};

// This function takes a list of primitive types as the template argument.
// It defines 3 arrays with information about them: type IDs, sizes and properties of the types.
// For example uint32_t, float64_t and const char * can be distinguished by their size and the property flags
// char, const, pointer, float and unsigned.
// Char is not treated as a simple int8_t because as a const pointer it can also identify a string literal.

template <typename... Args>
static const void **definePrimitiveTypes() {
	static TYPEID typeList[] = { Typer<Args>::makeID()..., nullptr };
	static const uint32_t sizeList[] = { sizeof(Args)... };
	static const uint8_t flagList[] = { (
		isSignless<Args>::value * 4 |
		// Type is floating point?
		(static_cast<Args>(1/2) != 0) * 2 |
		// Type is unsigned?
		(static_cast<Args>(-1) >= 0)
	)... };

	static const void *data[] = {
		typeList,
		sizeList,
		flagList
	};

	return(data);
}

// Linkage for lists of all C++ class and function wrappers.

std::forward_list<BindClassBase *> &nbind :: getClassList() {
	// This stops working if moved outside the function.
	static std::forward_list<BindClassBase *> classList;

	return(classList);
}

std::forward_list<MethodDef> &nbind :: getFunctionList() {
	// This stops working if moved outside the function.
	static std::forward_list<MethodDef> functionList;

	return(functionList);
}

const void **nbind :: getPrimitiveList() {
	static const void **primitiveList = definePrimitiveTypes<
		unsigned char,  signed char,    char,
		unsigned short, signed short,
		unsigned int,   signed int,
		unsigned long,  signed long,
		unsigned long long, signed long long,

		float, double
	>();

	return(primitiveList);
}

#define NBIND_TYPE(type) Typer<type>::makeID(), #type

const void **nbind :: getNamedTypeList() {
	static const void *typeList[] = {
		NBIND_TYPE(void),
		NBIND_TYPE(bool),
		NBIND_TYPE(std::string),
		NBIND_TYPE(cbFunction &),
		NBIND_TYPE(Buffer),
		nullptr
	};

	return(typeList);
}

void nbind :: registerClass(BindClassBase &spec) {
	getClassList().emplace_front(&spec);
}

void nbind :: registerFunction(
	const char *name,
	funcPtr ptr,
	unsigned int num,
	BaseSignature *signature,
	WrapperFlags flags
) {
	getFunctionList().emplace_front(name, ptr, num, signature, flags);
}

#include "nbind/nbind.h"

NBIND_CLASS(Int64) {}
