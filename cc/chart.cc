#include <cctype>
#include <cassert>
#include <map>

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

#include "chart.hh"

using namespace hidb;

// ----------------------------------------------------------------------

AntigenSerum::~AntigenSerum()
{

} // AntigenSerum::~AntigenSerum

// ----------------------------------------------------------------------

std::string Antigen::variant_id() const
{
    std::string n;
    if (is_reassortant()) {
        n.append(reassortant());
    }
    if (!annotations().empty()) {
        for (const auto& ann: annotations()) {
            if (!n.empty())
                n.append(1, ' ');
            n.append(ann);
        }
    }
    if (has_passage()) {
        if (!n.empty())
            n.append(1, ' ');
        n.append(passage());
    }

    return n;

} // Antigen::variant_id

// ----------------------------------------------------------------------

std::string Serum::variant_id() const
{
    std::string n;
    if (is_reassortant()) {
        n.append(reassortant());
    }
    if (!mSerumId.empty()) {
        if (!n.empty())
            n.append(1, ' ');
        n.append(mSerumId);
    }
    if (!annotations().empty()) {
        for (const auto& ann: annotations()) {
            if (!n.empty())
                n.append(1, ' ');
            n.append(ann);
        }
    }
    return n;

} // Serum::variant_id

// ----------------------------------------------------------------------

std::string AntigenSerum::passage_without_date() const
{
    if (mPassage.size() > 13 && mPassage[mPassage.size() - 1] == ')' && mPassage[mPassage.size() - 12] == '(' && mPassage[mPassage.size() - 13] == ' ' && mPassage[mPassage.size() - 4] == '-' && mPassage[mPassage.size() - 7] == '-')
        return std::string(mPassage, 0, mPassage.size() - 13);
    else
        return mPassage;

} // AntigenSerum::passage_without_date

// ----------------------------------------------------------------------

std::string Serum::full_name() const
{
    std::string n = name_for_exact_matching();
    if (has_passage()) {
        n.append(1, ' ');
        n.append(passage());
    }
    if (!mSerumSpecies.empty()) {
        n.append(1, ' ');
        n.append(mSerumSpecies);
    }
    return n;

} // Serum::full_name

// ----------------------------------------------------------------------

std::string Serum::name_for_exact_matching() const // full_name without passage, serum species
{
    std::string n = name();
    const auto vi = variant_id();
    if (!vi.empty()) {
        n.append(1, ' ');
        n.append(vi);
    }
    return n;

} // Serum::name_for_exact_matching

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

bool AntigenSerum::is_egg() const
{
    static std::regex egg_passage{
        R"#(E(\?|[0-9][0-9]?))#"  // passage
        R"#(( (ISOLATE|CLONE) [0-9\-]+)*)#"         // NIMR isolate and/or clone, NIMR H1pdm has CLONE 38-32
        R"#(( *\+[1-9])?)#"         // NIID has +1 at the end of passage
        R"#(( \([12][0129][0-9][0-9]-[01][0-9]-[0-3][0-9]\))?$)#" // passage date
       };
    return std::regex_search(mPassage, egg_passage) || is_reassortant(); // reassortant is always egg (2016-10-21)
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

bool Serum::is_egg() const
{
    bool egg = AntigenSerum::is_egg();
    if (!egg && !mSerumId.empty())
        egg = mSerumId.find("EGG") != std::string::npos; // NIID has EGG inside serum_id instead of passage
    return egg;

} // Serum::is_egg

// ----------------------------------------------------------------------

AntigenSerumMatch AntigenSerum::match(const AntigenSerum& aNother) const
{
      // fields not used for matching
      // lineage, semantic-attributes, annotations

    AntigenSerumMatch match;
    if (!distinct() && !aNother.distinct() && name() == aNother.name()) {
        if (reassortant() == aNother.reassortant()) {
            match.add(match_passage(aNother));
        }
        else {
            match.add(AntigenSerumMatch::ReassortantMismatch);
        }
    }
    else {
        match.add(AntigenSerumMatch::NameMismatch);
    }
    return match;

} // AntigenSerum::match

// ----------------------------------------------------------------------

AntigenSerumMatch AntigenSerum::match_passage(const AntigenSerum& aNother) const
{
    AntigenSerumMatch match;
    if (has_passage() && aNother.has_passage()) {
        if (passage() != aNother.passage()) {
            match.add(AntigenSerumMatch::PassageMismatch);
            if (passage_without_date() != aNother.passage_without_date()) {
                match.add(AntigenSerumMatch::PassageWithoutDateMismatch);
                  // std::cerr << "  ?egg " << is_egg() << "  " << full_name() << std::endl;
                  // std::cerr << "  ?egg " << aNother.is_egg() << "  " << aNother.full_name() << std::endl;
                if (is_egg() != aNother.is_egg()) {
                    match.add(AntigenSerumMatch::EggCellMismatch);
                }
            }
        }
    }
    else {
        match.add(AntigenSerumMatch::EggCellUnknown);
    }
    return match;

} // AntigenSerum::match_passage

// ----------------------------------------------------------------------

AntigenSerumMatch Antigen::match(const Antigen& aNother) const
{
      // fields not used for matching
      // date, clades, lab_id

    AntigenSerumMatch m = AntigenSerum::match(aNother);
    if (annotations() != aNother.annotations()) {
        m.add(AntigenSerumMatch::AnnotationMismatch);
    }
    return m;

} // Antigen::match

// ----------------------------------------------------------------------

AntigenSerumMatch Antigen::match(const Serum& aNother) const
{
    return aNother.match(*this);

} // Antigen::match

// ----------------------------------------------------------------------

AntigenSerumMatch Serum::match(const Serum& aNother) const
{
      // fields not used for matching
      // homologous

    AntigenSerumMatch m = AntigenSerum::match(aNother);
    if (m < AntigenSerumMatch::Mismatch) {
        if (annotations() != aNother.annotations()) {
            m.add(AntigenSerumMatch::AnnotationMismatch);
        }
        else {
            if (serum_id() != aNother.serum_id())
                m.add(AntigenSerumMatch::SerumIdMismatch);
            if (serum_species() != aNother.serum_species())
                m.add(AntigenSerumMatch::SerumSpeciesMismatch);
        }
    }
    return m;

} // Serum::match

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

AntigenSerumMatch Serum::match(const Antigen& aNother) const
{
    AntigenSerumMatch m = AntigenSerum::match(aNother);
    if (m < AntigenSerumMatch::Mismatch) {
                  // ignore serum specific annotations (CONC*, BOOSTED*, *BLEED)
        Annotations self_filtered;
        static std::regex serum_specific {"(CONC|BOOSTED|BLEED)"};
        static auto filter = [](const auto& anno) -> bool { return !std::regex_search(anno, serum_specific); };
        std::copy_if(annotations().begin(), annotations().end(), std::back_inserter(self_filtered), filter);
        if (self_filtered != aNother.annotations()) {
            m.add(AntigenSerumMatch::AnnotationMismatch);
        }
    }
    return m;

} // Serum::match

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

AntigenSerumMatch Serum::match_passage(const AntigenSerum& aNother) const
{
    AntigenSerumMatch match;
    if (!has_passage() && aNother.has_passage()) {
        if (is_egg() != aNother.is_egg()) {
            match.add(AntigenSerumMatch::EggCellMismatch);
        }
    }
    else {
        match = AntigenSerum::match_passage(aNother);
    }
    return match;

} // Serum::match_passage

// ----------------------------------------------------------------------

std::string Chart::lineage() const
{
    std::string lineage;
    if (mInfo.virus_type() == "B") {
        std::map<std::string, size_t> lineages;
        for (const auto& antigen: mAntigens)
            ++lineages[antigen.lineage()];
        if (lineages.size() == 1) {
            lineage = lineages.begin()->first;
        }
        else if (lineages.size() == 2) {
            auto e2 = lineages.begin();
            ++e2;
            auto cmp = [](const auto& a, const auto& b) { return a->second < b->second; };
            const auto minor_count = std::min(lineages.begin(), e2, cmp)->second;
            if (minor_count <= 3 && mAntigens.size() > 10)
                lineage = std::max(lineages.begin(), e2, cmp)->first;
        }
    }
    return lineage;

} // Chart::lineage

// ----------------------------------------------------------------------

void Chart::find_homologous_antigen_for_sera()
{
    for (auto& serum: mSera) {
        if (!serum.has_homologous()) { // it can be already set in .ace, e.g. manually during source excel sheet parsing
              // std::cout << serum.full_name() << std::endl;
            std::vector<std::pair<size_t, AntigenSerumMatch>> antigen_match;
            for (auto antigen = mAntigens.begin(); antigen != mAntigens.end(); ++antigen) {
                AntigenSerumMatch match{serum.match(*antigen)};
                if (match.name_match())
                    antigen_match.emplace_back(static_cast<size_t>(antigen - mAntigens.begin()), std::move(match));
            }
            switch (antigen_match.size()) {
              case 0:
                  std::cerr << "Warning: No homologous antigen (no name match at all) for " << serum.full_name() << std::endl;
                  break;
              case 1:
                  if (antigen_match.front().second.reassortant_match()) {
                      serum.set_homologous(antigen_match.front().first);
                  }
                  else {
                      std::cerr << "Warning: No homologous antigen for " << serum.full_name() << std::endl;
                      std::cerr << "    the only name match: " << mAntigens[antigen_match.front().first].full_name() << " Level:" << antigen_match.front().second << std::endl;
                  }
                  break;
              default:
                  // sort by AntigenSerumMatch but prefer reference antigens if matches are equal
                std::sort(antigen_match.begin(), antigen_match.end(), [this](const auto& a, const auto& b) -> bool { return a.second == b.second ? mAntigens[a.first].reference() > mAntigens[b.first].reference() : a.second < b.second; });
                if (antigen_match.front().second.homologous_match()) {
                    serum.set_homologous(antigen_match.front().first);
                    if (antigen_match.size() > 1 && antigen_match[0].second == antigen_match[1].second && mAntigens[antigen_match[0].first].reference() == mAntigens[antigen_match[1].first].reference()) {
                        std::cerr << "Warning: Multiple homologous antigen candidates for " << serum.full_name() << " (the first one chosen)" << std::endl;
                        for (const auto ag: antigen_match) {
                            if (ag.second != antigen_match.front().second)
                                break;
                            std::cerr << "    " << mAntigens[ag.first].full_name() << " Ref:" << mAntigens[ag.first].reference() << " Level:" << ag.second << std::endl;
                        }
                    }
                }
                else {
                    std::cerr << "Warning: No homologous antigen for " << serum.full_name() << std::endl;
                      // std::cerr << "    best match (of " << antigen_match.size() << "): " << mAntigens[antigen_match.front().first].full_name() << " Level:" << antigen_match.front().second << std::endl;
                    for (const auto ag: antigen_match) {
                        if (ag.second != antigen_match.front().second)
                            break;
                        std::cerr << "    " << mAntigens[ag.first].full_name() << " Ref:" << mAntigens[ag.first].reference() << " Level:" << ag.second << std::endl;
                    }
                }
                break;
            }
        }
    }

} // Chart::find_homologous_antigen_for_sera

// ----------------------------------------------------------------------

std::string ChartInfo::table_id(std::string lineage) const
{
    std::string date;
    if (mSources.empty()) {
        date = mDate;
    }
    else {
        date = mSources.front().mDate + "-" + mSources.back().mDate;
    }
    std::string assay = mAssay;
    if (assay == "FOCUS REDUCTION")
        assay = "FR";
    else if (assay == "PLAQUE REDUCTION NEUTRALISATION")
        assay = "PRN";
    std::string r = assay + ":" + mVirusType;
    if (!lineage.empty()) {
        r += ":" + lineage;
    }
    r += ":" + mLab;
    if (!mRbc.empty()) {
        r.append(1, ':');
        if (mRbc == "guinea-pig")
            r.append("gp");
        else if (mRbc == "turkey")
            r.append("tu");
        else if (mRbc == "chicken")
            r.append("ch");
        else
            r.append(mRbc);
    }
    if (!mName.empty())
        r += ":" + mName;
    if (!mDate.empty())
        r += ":" + mDate;
    std::transform(r.begin(), r.end(), r.begin(), [](auto c) { return std::tolower(c); });
    return r;

} // ChartInfo::table_id

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
