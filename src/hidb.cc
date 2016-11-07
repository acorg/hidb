#include <iomanip>

#include "hidb.hh"
#include "hidb-export.hh"
#include "string-matcher.hh"

// ----------------------------------------------------------------------

ChartData::ChartData(const Chart& aChart)
    : mTableId(aChart.table_id()), mTiters(aChart.titers().as_list())
{
    for (const auto& antigen: aChart.antigens()) {
        mAntigens.emplace_back(antigen.name(), antigen.variant_id());
    }
    for (const auto& serum: aChart.sera()) {
        mSera.emplace_back(serum.name(), serum.variant_id());
    }
}

// ----------------------------------------------------------------------

void HiDb::add(const Chart& aChart)
{
    ChartData chart(aChart);
    std::cout << chart.table_id() << std::endl;
    auto chart_insert_at = std::lower_bound(mCharts.begin(), mCharts.end(), chart);
    if (chart_insert_at != mCharts.end() && chart_insert_at->table_id() == chart.table_id())
        throw std::runtime_error("Chart " + chart.table_id() + " already in hidb");
    mCharts.insert(chart_insert_at, std::move(chart));

    aChart.find_homologous_antigen_for_sera_const();

    const auto table_id = aChart.table_id();
    for (const auto& antigen: aChart.antigens()) {
        add_antigen(antigen, table_id);
    }
    for (const auto& serum: aChart.sera()) {
        add_serum(serum, table_id, aChart.antigens());
    }

    // std::cout << "Chart: antigens:" << aChart.number_of_antigens() << " sera:" << aChart.number_of_sera() << std::endl;
    // std::cout << "HDb: antigens:" << mAntigens.size() << " sera:" << mSera.size() << std::endl;

} // HiDb::add

// ----------------------------------------------------------------------

void HiDb::add_antigen(const Antigen& aAntigen, std::string aTableId)
{
    if (!aAntigen.distinct()) {
        AntigenData antigen_data(aAntigen);
        auto insert_at = std::lower_bound(mAntigens.begin(), mAntigens.end(), aAntigen);
        if (insert_at != mAntigens.end() && *insert_at == antigen_data) {
              // update
              // std::cout << "Common antigen " << aAntigen.full_name() << std::endl;
        }
        else {
            insert_at = mAntigens.insert(insert_at, std::move(antigen_data));
        }
        insert_at->update(aTableId, aAntigen);
    }

} // HiDb::add_antigen

// ----------------------------------------------------------------------

void HiDb::add_serum(const Serum& aSerum, std::string aTableId, const std::vector<Antigen>& aAntigens)
{
    if (!aSerum.distinct()) {
        SerumData serum_data(aSerum);
        auto insert_at = std::lower_bound(mSera.begin(), mSera.end(), aSerum);
        if (insert_at != mSera.end() && *insert_at == serum_data) {
              // update
        }
        else {
            insert_at = mSera.insert(insert_at, std::move(serum_data));
        }
        insert_at->update(aTableId, aSerum);
        if (aSerum.has_homologous())
            insert_at->set_homologous(aTableId, aAntigens[static_cast<size_t>(aSerum.homologous())].variant_id());
    }

} // HiDb::add_serum

// ----------------------------------------------------------------------

void HiDb::exportTo(std::string aFilename, bool aPretty) const
{
    if (aPretty)
        hidb_export_pretty(aFilename, *this);
    else
        hidb_export(aFilename, *this);

} // HiDb::exportTo

// ----------------------------------------------------------------------

void HiDb::importFrom(std::string aFilename)
{
    hidb_import(aFilename, *this);

} // HiDb::importFrom

// ----------------------------------------------------------------------

class FindAntigenScore
{
 private:
    static constexpr const string_match::score_t keyword_in_lookup = 1;

 public:
    inline FindAntigenScore(std::string name, const AntigenData& aAntigen, string_match::score_t aNameScoreThreshold)
        : mAntigen(&aAntigen), mName(0), mFull(0)
        {
            const auto antigen_name = aAntigen.data().name();
            mName = string_match::match(antigen_name, name);
            if (aNameScoreThreshold == 0)
                aNameScoreThreshold = static_cast<string_match::score_t>(name.length() * name.length() * 0.05);
            if (mName >= aNameScoreThreshold) {
                const auto full_name = aAntigen.data().full_name();
                mFull = std::max({
                    for_subst(full_name, antigen_name.size(), name, " CELL", {" MDCK", " SIAT"}),
                    for_subst(full_name, antigen_name.size(), name, " EGG", {" E"}),
                    for_subst(full_name, antigen_name.size(), name, " REASSORTANT", {" NYMC", " IVR", " NIB", " RESVIR", " RG", " VI", " REASSORTANT"})
                    });
                if (mFull == 0)
                    mFull = string_match::match(full_name, name);
            }
        }

    inline bool operator < (const FindAntigenScore& aNother) const
        {
              // if mFull == keyword_in_lookup, move it to the end of the sorting list regardless of mName
            bool result;
            if (mFull == keyword_in_lookup)
                result = false;
            else if (aNother.mFull == keyword_in_lookup)
                result = true;
            else
                result = mName == aNother.mName ? mFull > aNother.mFull : mName > aNother.mName;
            return result;
        }

    inline bool operator == (const FindAntigenScore& aNother) const { return mName == aNother.mName; }
    inline operator const AntigenData*() const { return mAntigen; }
    inline operator bool() const { return mName > 0; }
    inline string_match::score_t name_score() const { return mName; }
    inline std::pair<const AntigenData*, size_t> antigen_score() const { return std::make_pair(mAntigen, mFull); }

 private:
    const AntigenData* mAntigen;
    string_match::score_t mName, mFull;

    inline string_match::score_t for_subst(std::string full_name, size_t name_part_size, std::string name, std::string keyword, std::initializer_list<std::string>&& subst_list)
    {
        string_match::score_t score = 0;
        const auto pos = name.find(keyword);
        if (pos != std::string::npos) { // keyword is in the lookup name
            for (const auto& subst: subst_list) {
                if (full_name.find(subst.c_str() + 1, name_part_size) != std::string::npos) { // subst (without leading space) must be present in full_name in the passage part
                    std::string substituted(name, 0, pos);
                    substituted.append(subst);
                    score = std::max(score, string_match::match(full_name, substituted));
                }
                else if (score == 0)
                    score = keyword_in_lookup; // to avoid using name-with-keyword-not-replaced for matching
            }
        }
        return score;
    }
};

// ----------------------------------------------------------------------

inline static void find_scores(std::string name, const std::vector<AntigenData>& antigens, std::vector<FindAntigenScore>& scores, std::vector<FindAntigenScore>::iterator& scores_end)
{
    string_match::score_t score_threshold = 0;
    for (const auto& antigen: antigens) {
        scores.emplace_back(name, antigen, score_threshold);
        score_threshold = std::max(scores.back().name_score(), score_threshold);
    }
    std::sort(scores.begin(), scores.end());
    scores_end = std::find_if_not(scores.begin(), scores.end(), [&scores](const auto& e) { return e == scores.front(); }); // just use entries with maximal name score

} // HiDb::find_scores

// ----------------------------------------------------------------------

std::vector<const AntigenData*> HiDb::find_antigens(std::string name) const
{
    std::vector<FindAntigenScore> scores;
    std::vector<FindAntigenScore>::iterator scores_end;
    find_scores(name, antigens(), scores, scores_end);
    return {scores.begin(), scores_end};

} // HiDb::find_antigens

// ----------------------------------------------------------------------

std::vector<std::pair<const AntigenData*, size_t>> HiDb::find_antigens_with_score(std::string name) const
{
    std::vector<FindAntigenScore> scores;
    std::vector<FindAntigenScore>::iterator scores_end;
    find_scores(name, antigens(), scores, scores_end);
    std::vector<std::pair<const AntigenData*, size_t>> result;
    std::transform(scores.begin(), scores_end, std::back_inserter(result), [](const auto& e) { return e.antigen_score(); });
    return result;

} // HiDb::find_antigens_with_score

// ----------------------------------------------------------------------

std::vector<std::string> HiDb::list_antigens() const
{
    std::vector<std::string> result;
    std::transform(antigens().begin(), antigens().end(), std::back_inserter(result), [](const auto& ag) -> std::string { return ag.data().name(); });
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;

} // HiDb::list_antigens

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
