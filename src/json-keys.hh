#pragma once

// ----------------------------------------------------------------------

// Keys work for both chart in the ace format and hidb

enum class JsonKey : char
{
    Comment='?', Comment_='_',
    Chart='c', Antigens='a', Sera='s', Info='i', Projections='P', PlotSpec='p', Titers='t',
    Assay='A', Virus='v', VirusType='V', Date='D', Name='N', Lab='l', Rbc='r', VirusSubset='s', TableType='T', Sources='S',
    Lineage='L', Passage='P', Reassortant='R', LabId='l', Annotations='a', Clades='c', SemanticAttributes='S',
    SerumId='I', HomologousAntigen='h', SerumSpecies='s',
    TitersList='l', TitersDict='d', TitersLayers='L',
    DrawingOrder='d', ErrorLinePositive='E', ErrorLineNegative='e', Grid='g', PointIndex='p', PointStyles='P', ProcrustesIndex='l', ProcrustesStyle='L', ShownOnAll='s', Title='t',
    ColumnBases='C',
      // HiDb
    Tables='t', TableId='T', PerTable='T',
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
