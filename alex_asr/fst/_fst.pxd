cimport sym
cimport libfst


cdef class _Fst:
    pass 

cdef class SymbolTable:
    cdef sym.SymbolTable* table


cdef class StdVectorFst(_Fst):
    cdef libfst.StdVectorFst* fst
    cdef public SymbolTable isyms, osyms


cdef class LogVectorFst(_Fst):
    cdef libfst.LogVectorFst* fst
    cdef public SymbolTable isyms, osyms

