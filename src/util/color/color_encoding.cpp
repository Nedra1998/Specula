#include "util/color/color_encoding.hpp"

#include <cstdlib>
#include <map>

#include "util/string.hpp"

SPECULA_CONST specula::Float specula::SRGB_TO_LINEAR_LUT[256] = {
    0.0000000000, 0.0003035270, 0.0006070540, 0.0009105810, 0.0012141080, 0.0015176350,
    0.0018211619, 0.0021246888, 0.0024282159, 0.0027317430, 0.0030352699, 0.0033465356,
    0.0036765069, 0.0040247170, 0.0043914421, 0.0047769533, 0.0051815170, 0.0056053917,
    0.0060488326, 0.0065120910, 0.0069954102, 0.0074990317, 0.0080231922, 0.0085681248,
    0.0091340570, 0.0097212177, 0.0103298230, 0.0109600937, 0.0116122449, 0.0122864870,
    0.0129830306, 0.0137020806, 0.0144438436, 0.0152085144, 0.0159962922, 0.0168073755,
    0.0176419523, 0.0185002182, 0.0193823613, 0.0202885624, 0.0212190095, 0.0221738834,
    0.0231533647, 0.0241576303, 0.0251868572, 0.0262412224, 0.0273208916, 0.0284260381,
    0.0295568332, 0.0307134409, 0.0318960287, 0.0331047624, 0.0343398079, 0.0356013142,
    0.0368894450, 0.0382043645, 0.0395462364, 0.0409151986, 0.0423114114, 0.0437350273,
    0.0451862030, 0.0466650836, 0.0481718220, 0.0497065634, 0.0512694679, 0.0528606549,
    0.0544802807, 0.0561284944, 0.0578054339, 0.0595112406, 0.0612460710, 0.0630100295,
    0.0648032799, 0.0666259527, 0.0684781820, 0.0703601092, 0.0722718611, 0.0742135793,
    0.0761853904, 0.0781874284, 0.0802198276, 0.0822827145, 0.0843762159, 0.0865004659,
    0.0886556059, 0.0908417329, 0.0930589810, 0.0953074843, 0.0975873619, 0.0998987406,
    0.1022417471, 0.1046164930, 0.1070231125, 0.1094617173, 0.1119324341, 0.1144353822,
    0.1169706732, 0.1195384338, 0.1221387982, 0.1247718409, 0.1274376959, 0.1301364899,
    0.1328683347, 0.1356333494, 0.1384316236, 0.1412633061, 0.1441284865, 0.1470272839,
    0.1499598026, 0.1529261619, 0.1559264660, 0.1589608639, 0.1620294005, 0.1651322246,
    0.1682693958, 0.1714410931, 0.1746473908, 0.1778884083, 0.1811642349, 0.1844749898,
    0.1878207624, 0.1912016720, 0.1946178079, 0.1980693042, 0.2015562356, 0.2050787061,
    0.2086368501, 0.2122307271, 0.2158605307, 0.2195262313, 0.2232279778, 0.2269658893,
    0.2307400703, 0.2345506549, 0.2383976579, 0.2422811985, 0.2462013960, 0.2501583695,
    0.2541521788, 0.2581829131, 0.2622507215, 0.2663556635, 0.2704978585, 0.2746773660,
    0.2788943350, 0.2831487954, 0.2874408960, 0.2917706966, 0.2961383164, 0.3005438447,
    0.3049873710, 0.3094689548, 0.3139887452, 0.3185468316, 0.3231432438, 0.3277781308,
    0.3324515820, 0.3371636569, 0.3419144452, 0.3467040956, 0.3515326977, 0.3564002514,
    0.3613068759, 0.3662526906, 0.3712377846, 0.3762622178, 0.3813261092, 0.3864295185,
    0.3915725648, 0.3967553079, 0.4019778669, 0.4072403014, 0.4125427008, 0.4178851545,
    0.4232677519, 0.4286905527, 0.4341537058, 0.4396572411, 0.4452012479, 0.4507858455,
    0.4564110637, 0.4620770514, 0.4677838385, 0.4735315442, 0.4793202281, 0.4851499796,
    0.4910208881, 0.4969330430, 0.5028865933, 0.5088814497, 0.5149177909, 0.5209956765,
    0.5271152258, 0.5332764983, 0.5394796133, 0.5457245708, 0.5520114899, 0.5583404899,
    0.5647116303, 0.5711249113, 0.5775805116, 0.5840784907, 0.5906189084, 0.5972018838,
    0.6038274169, 0.6104956269, 0.6172066331, 0.6239604354, 0.6307572126, 0.6375969648,
    0.6444797516, 0.6514056921, 0.6583748460, 0.6653873324, 0.6724432111, 0.6795425415,
    0.6866854429, 0.6938719153, 0.7011020184, 0.7083759308, 0.7156936526, 0.7230552435,
    0.7304608822, 0.7379105687, 0.7454043627, 0.7529423237, 0.7605246305, 0.7681512833,
    0.7758223414, 0.7835379243, 0.7912980318, 0.7991028428, 0.8069523573, 0.8148466945,
    0.8227858543, 0.8307699561, 0.8387991190, 0.8468732834, 0.8549926877, 0.8631572723,
    0.8713672161, 0.8796223402, 0.8879231811, 0.8962693810, 0.9046613574, 0.9130986929,
    0.9215820432, 0.9301108718, 0.9386858940, 0.9473065734, 0.9559735060, 0.9646862745,
    0.9734454751, 0.9822505713, 0.9911022186, 1.0000000000};

void specula::ColorEncoding::initialize(Allocator allocator) {
  LINEAR = allocator.new_object<LinearColorEncoding>();
  SRGB = allocator.new_object<sRgbColorEncoding>();
}

specula::ColorEncoding specula::ColorEncoding::LINEAR;
specula::ColorEncoding specula::ColorEncoding::SRGB;

const specula::ColorEncoding specula::ColorEncoding::get(const std::string &name,
                                                         Allocator allocator) {
  if (name == "linear") {
    return ColorEncoding::LINEAR;
  } else if (name == "srgb" || name == "sRGB") {
    return ColorEncoding::SRGB;
  } else {
    static std::map<float, ColorEncoding> cache;
    static std::mutex mutex;

    std::vector<std::string> params = split_strings_from_whitespace(name);
    if (params.size() != 2 || params[0] != "gamma") {
      LOG_CRITICAL("{}: expected \"gamma <value>\" for color encoding", name);
      // TODO: Better exit handling for OSX
      std::quick_exit(1);
    }
    Float gamma = atof(params[1].c_str());
    if (gamma == 0) {
      LOG_CRITICAL("{}: unable to parse gamma value", params[1]);
      // TODO: Better exit handling for OSX
      std::quick_exit(1);
    }

    std::lock_guard<std::mutex> lock(mutex);
    auto iter = cache.find(gamma);
    if (iter != cache.end()) {
      return iter->second;
    }

    ColorEncoding enc = allocator.new_object<GammaColorEncoding>(gamma);
    cache[gamma] = enc;
    LOG_DEBUG("Added ColorEncoding {} for gamma {} -> {}", name, gamma, enc);
    return enc;
  }
}

SPECULA_CPU_GPU inline void specula::sRgbColorEncoding::to_linear(pstd::span<const uint8_t> vin,
                                                                  pstd::span<Float> vout) const {

  DASSERT_EQ(vin.size(), vout.size());
  for (size_t i = 0; i < vin.size(); ++i) {
    vout[i] = srgb8_to_linear(vin[i]);
  }
}

SPECULA_CPU_GPU void specula::sRgbColorEncoding::from_linear(pstd::span<const Float> vin,
                                                             pstd::span<uint8_t> vout) const {
  DASSERT_EQ(vin.size(), vout.size());
  for (size_t i = 0; i < vin.size(); ++i) {
    vout[i] = linear_to_srgb8(vin[i]);
  }
}

SPECULA_CPU_GPU inline specula::Float specula::sRgbColorEncoding::to_float_linear(Float v) const {
  return srgb_to_linear(v);
}

SPECULA_CPU_GPU specula::GammaColorEncoding::GammaColorEncoding(Float gamma) : gamma(gamma) {
  for (int i = 0; i < 256; ++i) {
    Float v = Float(i) / 255.0f;
    apply_lut[i] = std::pow(v, gamma);
  }

  for (int i = 0; i < int(inverse_lut.size()); ++i) {
    Float v = Float(i) / (inverse_lut.size() - 1);
    inverse_lut[i] = clamp(255.0f * std::pow(v, 1.0f / gamma) + 0.5f, 0, 255);
  }
}

SPECULA_CPU_GPU inline void specula::GammaColorEncoding::to_linear(pstd::span<const uint8_t> vin,
                                                                   pstd::span<Float> vout) const {

  DASSERT_EQ(vin.size(), vout.size());
  for (size_t i = 0; i < vin.size(); ++i) {
    vout[i] = apply_lut[vin[i]];
  }
}

SPECULA_CPU_GPU void specula::GammaColorEncoding::from_linear(pstd::span<const Float> vin,
                                                              pstd::span<uint8_t> vout) const {
  DASSERT_EQ(vin.size(), vout.size());
  for (size_t i = 0; i < vin.size(); ++i) {
    vout[i] = inverse_lut[clamp(vin[i] * (inverse_lut.size() - 1), 0, inverse_lut.size() - 1)];
  }
}

SPECULA_CPU_GPU inline specula::Float specula::GammaColorEncoding::to_float_linear(Float v) const {
  return std::pow(v, gamma);
}
