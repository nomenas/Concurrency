#include <iostream>

extern void test_interface_callback_safety();
extern void test_safe_local_members_asynchronous();
extern void test_locals_using_shared_pointers();
extern void test_locals_using_weak_pointer_checker();
extern void test_proxy_thru_object_accessor();

int main() {
    std::cout << "=== test_interface_callback_safety ===" << std::endl;
    test_interface_callback_safety();

    std::cout << "\n=== test_locals_using_shared_pointers ===" << std::endl;
    test_locals_using_shared_pointers();

    std::cout << "\n=== test_locals_using_weak_pointer_checker ===" << std::endl;
    test_locals_using_weak_pointer_checker();

    std::cout << "\n=== test_safe_local_members_asynchronous ===" << std::endl;
    test_safe_local_members_asynchronous();

    std::cout << "\n=== test_proxy_thru_object_accessor ===" << std::endl;
    test_proxy_thru_object_accessor();

    return 0;
}