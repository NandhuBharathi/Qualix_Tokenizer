#include <iostream>
#include <vector>

#include "bpe/model.hpp"
#include "bpe/serializer.hpp"

using namespace qualix;
using namespace qualix::bpe;

namespace
{

usize tests_run = 0;
usize tests_passed = 0;

void Expect(
    bool condition,
    const char* name
)
{
    ++tests_run;

    if (condition)
    {
        ++tests_passed;

        std::cout
            << "[PASS] "
            << name
            << '\n';
    }
    else
    {
        std::cout
            << "[FAIL] "
            << name
            << '\n';
    }
}

}

int main()
{
    /*
     * Canonical model must survive serialization.
     */
    {
        BpeModel model;

        auto& vocabulary =
            model.GetVocabulary();

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId ab =
            vocabulary.AddMerged(a, b);

        model.SetRules(
            {
                MergeRule{
                    Pair{a, b},
                    ab,
                    0
                }
            }
        );

        const auto serialized =
            BpeModelSerializer::Serialize(
                model
            );

        Expect(
            serialized.Ok(),
            "Canonical model serializes"
        );

        if (serialized.Ok())
        {
            const auto loaded =
                BpeModelSerializer::Deserialize(
                    serialized.Value()
                );

            Expect(
                loaded.Ok(),
                "Canonical model deserializes"
            );
        }
    }

    /*
     * Construct an in-memory model whose merge
     * target exists but has incorrect bytes.
     *
     * Serialization currently permits an existing
     * target ID. Deserialization must reject the
     * semantic corruption.
     */
    {
        BpeModel model;

        auto& vocabulary =
            model.GetVocabulary();

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId wrong =
            vocabulary.Add("wrong");

        model.SetRules(
            {
                MergeRule{
                    Pair{a, b},
                    wrong,
                    0
                }
            }
        );

        const auto serialized =
            BpeModelSerializer::Serialize(
                model
            );

        Expect(
            serialized.Ok(),
            "Structurally valid corrupt model serializes"
        );

        if (serialized.Ok())
        {
            const auto loaded =
                BpeModelSerializer::Deserialize(
                    serialized.Value()
                );

            Expect(
                loaded.Failed(),
                "Wrong merged bytes rejected on load"
            );
        }
    }

    /*
     * Rank must match canonical rule order.
     */
    {
        BpeModel model;

        auto& vocabulary =
            model.GetVocabulary();

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId ab =
            vocabulary.AddMerged(a, b);

        model.SetRules(
            {
                MergeRule{
                    Pair{a, b},
                    ab,
                    7
                }
            }
        );

        const auto serialized =
            BpeModelSerializer::Serialize(
                model
            );

        Expect(
            serialized.Ok(),
            "Noncanonical rank model serializes"
        );

        if (serialized.Ok())
        {
            const auto loaded =
                BpeModelSerializer::Deserialize(
                    serialized.Value()
                );

            Expect(
                loaded.Failed(),
                "Noncanonical merge rank rejected"
            );
        }
    }

    std::cout
        << '\n'
        << tests_passed
        << "/"
        << tests_run
        << " tests passed\n";

    return
        tests_passed == tests_run
            ? 0
            : 1;
}
