#include <oink_judge/test_node/invoker_protocol.h>
#include <oink_judge/test_node/problem_builders/default_problem_builder.h>
#include <oink_judge/test_node/tests/compilation_test.h>
#include <oink_judge/test_node/tests/single_test.h>
#include <oink_judge/test_node/tests/sync_result_test.h>
#include <oink_judge/test_node/tests/testset.h>
#include <oink_judge/test_node/verdict_aggregators/aggregate_current.h>
#include <oink_judge/test_node/verdict_aggregators/aggregate_max.h>
#include <oink_judge/test_node/verdict_builders/verdict_builder_min.h>
#include <oink_judge/test_node/verdict_builders/verdict_builder_sum.h>

using namespace oink_judge::test_node;

extern "C" void registerTypes() {
    registerDefaultProblemBuilderType();
    registerCompilationTestType();
    registerSingleTestType();
    registerSyncResultTestType();
    registerTestsetType();
    registerAggregateCurrentType();
    registerAggregateMaxType();
    registerVerdictBuilderMinType();
    registerVerdictBuilderSumType();
    registerInvokerProtocolType();
}
