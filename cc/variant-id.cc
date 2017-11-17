#include "variant-id.hh"

// ----------------------------------------------------------------------

std::string hidb::table_id(const acmacs::chart::Chart& aChart)
{
    auto info = aChart.info();

    std::string assay = info->assay();
    if (assay == "FOCUS REDUCTION")
        assay = "FR";
    else if (assay == "PLAQUE REDUCTION NEUTRALISATION")
        assay = "PRN";
    std::string r = assay + ":" + info->virus_type();
    const std::string lineage = aChart.lineage();
    if (!lineage.empty()) {
        r += ":" + lineage;
    }
    r += ":" + info->lab();
    const std::string rbc = info->rbc_species();
    if (!rbc.empty()) {
        r.append(1, ':');
        if (rbc == "guinea-pig")
            r.append("gp");
        else if (rbc == "turkey")
            r.append("tu");
        else if (rbc == "chicken")
            r.append("ch");
        else
            r.append(rbc);
    }
    const std::string name = info->name();
    if (!name.empty())
        r += ":" + name;
    const std::string date = info->date();
    if (!date.empty())
        r += ":" + date;
    std::transform(r.begin(), r.end(), r.begin(), [](auto c) { return std::tolower(c); });
    return r;

} // hidb::table_id

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
