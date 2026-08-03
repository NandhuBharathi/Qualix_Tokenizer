
#include <iostream>
#include <string>
#include <vector>

#include "bpe/model_trainer.hpp"
#include "bpe/serializer.hpp"

using namespace qualix;
using namespace qualix::bpe;

int main()
{
    const std::string corpus =
        "hello hello world world "
        "தமிழ் தமிழ் வணக்கம் வணக்கம் "
        "namma namma tokenizer tokenizer "
        "abab baba abab baba";

    TrainerConfig config;
    config.max_merges = 64;
    config.min_frequency = 2;

    auto first =
        BpeModelTrainer::Train(
            corpus,
            config
        );

    if (first.Failed())
    {
        std::cout << "[FAIL] first training\n";
        return 1;
    }

    auto second =
        BpeModelTrainer::Train(
            corpus,
            config
        );

    if (second.Failed())
    {
        std::cout << "[FAIL] second training\n";
        return 1;
    }

    auto third =
        BpeModelTrainer::Train(
            corpus,
            config
        );

    if (third.Failed())
    {
        std::cout << "[FAIL] third training\n";
        return 1;
    }

    auto& a = first.Value();
    auto& b = second.Value();
    auto& c = third.Value();

    if (a.VocabularySize() != b.VocabularySize() ||
        a.VocabularySize() != c.VocabularySize())
    {
        std::cout << "[FAIL] vocabulary size differs\n";
        return 1;
    }

    if (a.GetRules() != b.GetRules() ||
        a.GetRules() != c.GetRules())
    {
        std::cout << "[FAIL] merge rules differ\n";
        return 1;
    }

    auto sa =
        BpeModelSerializer::Serialize(a);

    auto sb =
        BpeModelSerializer::Serialize(b);

    auto sc =
        BpeModelSerializer::Serialize(c);

    if (sa.Failed() ||
        sb.Failed() ||
        sc.Failed())
    {
        std::cout << "[FAIL] serialization\n";
        return 1;
    }

    if (sa.Value() != sb.Value() ||
        sa.Value() != sc.Value())
    {
        std::cout << "[FAIL] binary models differ\n";
        return 1;
    }

    auto ea = a.Encode(corpus);
    auto eb = b.Encode(corpus);
    auto ec = c.Encode(corpus);

    if (ea.Failed() ||
        eb.Failed() ||
        ec.Failed())
    {
        std::cout << "[FAIL] inference\n";
        return 1;
    }

    if (ea.Value() != eb.Value() ||
        ea.Value() != ec.Value())
    {
        std::cout << "[FAIL] encoded IDs differ\n";
        return 1;
    }

    std::cout
        << "[PASS] deterministic vocabulary\n"
        << "[PASS] deterministic merge rules\n"
        << "[PASS] byte-identical serialization\n"
        << "[PASS] deterministic token IDs\n"
        << "Vocabulary : "
        << a.VocabularySize()
        << '\n'
        << "Rules      : "
        << a.RuleCount()
        << '\n';

    return 0;
}
