.\sourcerer\bin\debug\net7.0\sourcerer.exe `
    make_editorconfig `
    --dry_run:false `
    --project:astronomy `
    --project:base `
    --project:benchmarks `
    --project:geometry `
    --project:integrators `
    --project:journal `
    --project:ksp_plugin `
    --project:ksp_plugin_test `
    --project:mathematica `
    --project:numerics `
    --project:physics `
    --project:quantities `
    --project:testing_utilities `
    --project:tools `
    --extra:gmock/gmock.h=gmock/gmock-actions.h+gmock/gmock-matchers.h+gmock/gmock-nice-strict.h+gmock/gmock-spec-builders.h `
    --extra:gtest/gtest.h=gtest/gtest-matchers.h+gtest/internal/gtest-internal.h `
    --solution:.