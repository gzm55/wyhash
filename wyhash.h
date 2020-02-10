// Author: Wang Yi <godspeed_china@yeah.net>
#ifndef wyhash_version_5
#define wyhash_version_5
#include <stdint.h>
#include <string.h>
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#pragma intrinsic(_umul128)
#endif
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
#define _likely_(x) __builtin_expect(x,1)
#define _unlike_(x) __builtin_expect(x,0)
#else
#define _likely_(x) (x)
#define _unlike_(x) (x)
#endif
static inline uint64_t _wyrotr(uint64_t v, unsigned k){ return (v>>k)|(v<<(64-k)); }
static inline uint64_t _wymum(uint64_t A, uint64_t B){
// uint64_t  hh=(A>>32)*(B>>32), hl=(A>>32)*(unsigned)B, lh=(unsigned)A*(B>>32), ll=(uint64_t)(unsigned)A*(unsigned)B;
// return _wyrotr(hl,32)^_wyrotr(lh,32)^hh^ll;
#ifdef __SIZEOF_INT128__
 __uint128_t r=A; r*=B; return (r>>64)^r;
#elif defined(_MSC_VER) && defined(_M_X64)
 A=_umul128(A, B, &B); return A^B;
#else
 uint64_t ha=A>>32, hb=B>>32, la=(uint32_t)A, lb=(uint32_t)B, hi, lo;
 uint64_t rh=ha*hb, rm0=ha*lb, rm1=hb*la, rl=la*lb, t=rl+(rm0<<32), c=t<rl;
 lo=t+(rm1<<32); c+=lo<t; hi=rh+(rm0>>32)+(rm1>>32)+c; return hi^lo;
#endif
}
static inline uint64_t _wymix(const uint64_t A, const uint64_t B) { return _wyrotr(A-B,43)^_wymum(A,B); }
static inline uint64_t wyrand(uint64_t *seed){ *seed+=0xa0761d6478bd642full; return _wymum(*seed^0xe7037ed1a0b428dbull,*seed); }
static inline double wy2u01(uint64_t r){ const double _wynorm=1.0/(1ull<<52); return (r>>11)*_wynorm; }
static inline double wy2gau(uint64_t r){ const double _wynorm=1.0/(1ull<<20); return ((r&0x1fffff)+((r>>21)&0x1fffff)+((r>>42)&0x1fffff))*_wynorm-3.0; }
#ifndef WYHASH_LITTLE_ENDIAN
#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define WYHASH_LITTLE_ENDIAN 1
#elif defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define WYHASH_LITTLE_ENDIAN 0
#endif
#endif
#if(WYHASH_LITTLE_ENDIAN)
static inline uint64_t _wyr8(const uint8_t *p){ uint64_t v; memcpy(&v, p, 8); return v; }
static inline uint64_t _wyr4(const uint8_t *p){ unsigned v; memcpy(&v, p, 4); return v; }
#else
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
static inline uint64_t _wyr8(const uint8_t *p){ uint64_t v; memcpy(&v, p, 8); return  __builtin_bswap64(v); }
static inline uint64_t _wyr4(const uint8_t *p){ unsigned v; memcpy(&v, p, 4); return  __builtin_bswap32(v); }
#elif defined(_MSC_VER)
static inline uint64_t _wyr8(const uint8_t *p){ uint64_t v; memcpy(&v, p, 8); return _byteswap_uint64(v);}
static inline uint64_t _wyr4(const uint8_t *p){ unsigned v; memcpy(&v, p, 4); return _byteswap_ulong(v); }
#endif
#endif
static inline uint64_t _wyr3(const uint8_t *p, unsigned k){ return (((uint64_t)p[0])<<16)|(((uint64_t)p[k>>1])<<8)|p[k-1]; }
static inline uint64_t FastestHash(const void *key, size_t len){
 const uint8_t *p=(const uint8_t*)key;
 if(len>=8) return _wymum(_wyr8(p),_wyr8(p+len-8));
 else if(_likely_(len>=4)) return _wymum(_wyr4(p),_wyr4(p+len-4));
 else if(_likely_(len)) return _wymum(_wyr3(p,len),_wyr3(p,len));
 else return 0;
}
static inline uint64_t _wyhash(const void* key, const uint64_t len, uint64_t seed, const uint64_t secret[6]){
 const uint8_t *p=(const uint8_t*)key; uint64_t i=len; seed^=secret[4];
 if(_unlike_(i>64)){
  uint64_t see1=seed, see2=seed, see3=seed;
  for(; _likely_(i>=64); i-=64,p+=64){
   seed=_wymix(_wyr8(p)^secret[0],_wyr8(p+8)^seed);  see1=_wymix(_wyr8(p+16)^secret[1],_wyr8(p+24)^see1);
   see2=_wymix(_wyr8(p+32)^secret[2],_wyr8(p+40)^see2); see3=_wymix(_wyr8(p+48)^secret[3],_wyr8(p+56)^see3);
  }
  seed^=see1^see2^see3;
 }
 if(_likely_(i>=8)){
  if(_likely_(i<=16)) return _wymix(_wyr8(p)^secret[0],_wyr8(p+i-8)^seed);
  else if(_likely_(i<=32)) return _wymix(_wyr8(p)^secret[0],_wyr8(p+8)^seed)^_wymix(_wyr8(p+i-16)^secret[1],_wyr8(p+i-8)^seed);
  else return _wymix(_wyr8(p)^secret[0],_wyr8(p+8)^seed)^_wymix(_wyr8(p+16)^secret[1],_wyr8(p+24)^seed)
   ^_wymix(_wyr8(p+i-32)^secret[2],_wyr8(p+i-24)^seed)^_wymix(_wyr8(p+i-16)^secret[3],_wyr8(p+i-8)^seed);
 }
 else {
  if(_likely_(i>=4)) return _wymix(_wyr4(p)^secret[0],_wyr4(p+i-4)^seed);
  else if (_likely_(i)) return _wymix(_wyr3(p,i)^secret[0],seed);
  else return seed;
 }
}
static inline uint64_t wyhash(const void* key, uint64_t len, uint64_t seed, const uint64_t secret[6]){ return _wymum(_wyhash(key,len,seed,secret), (len^secret[5])|(uint32_t)0x80000000u); }
static inline void make_secret(uint64_t seed, uint64_t secret[6]){
 uint8_t c[]= {15,23,27,29,30,39,43,45,46,51,53,54,57,58,60,71,75,77,78,83,85,86,89,90,92,99,101,102,105,106,108,113,114,116,120,135,139,141,142,147,149,150,153,154,156,163,165,166,169,170,172,177,178,180,184,195,197,198,201,202,204,209,210,212,216,225,226,228,232,240};
 for(size_t i=0; i<6; i++){
  uint8_t ok;
  do{
   ok=1; secret[i]=0;
   for(size_t j=0; j<64; j+=8) secret[i]|=((uint64_t)c[wyrand(&seed)%sizeof(c)])<<j;
   for(size_t j=0; j<i; j++)
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
   if(__builtin_popcountll(secret[i]^secret[j])!=32) ok=0;
#elif defined(_MSC_VER)
   if(_mm_popcnt_u64(secret[i]^secret[j])!=32) ok=0;
#endif
  }while(!ok);
 }
}
static inline uint64_t wyhash64(uint64_t A, uint64_t B){ return _wymum(_wymum(A^0xa0761d6478bd642full,B^0xe7037ed1a0b428dbull),0x8ebc6af09c88c6e3ull); }
#endif
