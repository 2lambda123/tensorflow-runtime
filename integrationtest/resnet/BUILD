load("@tf_runtime//tools:mlir_to_bef.bzl", "glob_tfrt_lit_tests")

licenses(["notice"])

glob_tfrt_lit_tests(
    data = [
        # copybara:uncomment_begin(OSS TFRT user need to manually download the BTF file)
        # "test_data/resnet50_graph_train_tensors.btf",
        # "test_data/resnet_toy_tensors.btf",
        # ":generate_resnet50_eager_inference_input",
        # copybara:uncomment_end
        ":test_utilities",
    ],
    # Use size_override if needed.
    default_size = "large",
    default_tags = [
        "integration_test",
        "do_not_disable_rtti",
    ],
    exclude = [
        "*.benchmarks.mlir",
    ],
)

# copybara:uncomment_begin
# genrule(
#     name = "generate_resnet50_eager_inference_input",
#     srcs = ["test_data/imagenet.tfrecord"],
#     outs = ["resnet50_eager_inference_tensors.btf"],
#     cmd = """
#     $(location //third_party/tf_runtime/utils/resnet:resnet50_eager_inference_main.par) \
#       --mlir_file=dummy.mlir --weights_btf_file=$(OUTS) --data_dir=$(SRCS) \
#       --batch_size=1 --mode=correctness
#     """,
#     tools = ["@tf_runtime//utils/resnet:resnet50_eager_inference_main.par"],
# )
# copybara:uncomment_end

# Bundle together all of the test utilities that are used by tests.
filegroup(
    name = "test_utilities",
    testonly = True,
    srcs = [
        "@llvm-project//llvm:FileCheck",
        "@tf_runtime//tools:bef_executor",
    ],
)
