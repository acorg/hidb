#include "hidb.hh"

// ----------------------------------------------------------------------

void HiDb::add(const Chart& aChart)
{
    const auto table_id = aChart.table_id();
    for (const auto& antigen: aChart.antigens()) {
        add_antigen(antigen, table_id);
    }
    for (const auto& serum: aChart.sera()) {
        add_serum(serum, table_id);
    }
    auto insert_at = std::lower_bound(mTables.begin(), mTables.end(), aChart);
    mTables.insert(insert_at, aChart);

    std::cout << "Chart: antigens:" << aChart.number_of_antigens() << " sera:" << aChart.number_of_sera() << std::endl;
    std::cout << "HDb: antigens:" << mAntigens.size() << " sera:" << mSera.size() << std::endl;

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

void HiDb::add_serum(const Serum& aSerum, std::string aTableId)
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
    }

} // HiDb::add_serum

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
