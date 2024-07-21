/*
 * Exports ijl15 symbols.
 */

#ifdef __MINGW32__

unsigned long long __stdcall _aullshr(unsigned long long a, long b)
{
    return a >> b;
}

#endif
