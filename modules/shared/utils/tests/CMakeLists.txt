# ------------------------------------------------------------------------------
# Utils tests
# ------------------------------------------------------------------------------

add_module_test(oink_judge_utils_crypto_tests test_crypto.cpp oink_judge::utils)
add_module_test(oink_judge_utils_filesystem_tests test_filesystem.cpp
                oink_judge::utils)
