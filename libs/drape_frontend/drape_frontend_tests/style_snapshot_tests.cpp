#include "testing/testing.hpp"

#include "drape_frontend/stylist.hpp"

#include "indexer/classificator.hpp"
#include "indexer/classificator_loader.hpp"
#include "indexer/feature.hpp"
#include "indexer/map_object.hpp"
#include "indexer/map_style_reader.hpp"

#include <algorithm>
#include <array>
#include <latch>
#include <thread>

namespace style_snapshot_tests
{
std::array<MapStyle, 6> constexpr kStyles = {MapStyleDefaultLight, MapStyleDefaultDark,   MapStyleVehicleLight,
                                             MapStyleVehicleDark,  MapStyleOutdoorsLight, MapStyleOutdoorsDark};

class SampleFeature : public osm::MapObject
{
public:
  SampleFeature(uint32_t type, feature::GeomType geomType)
  {
    m_geomType = geomType;
    m_points = {{0.0, 0.0}, {0.01, 0.01}};
    m_triangles = {{0.0, 0.0}, {0.01, 0.0}, {0.0, 0.01}};
    m_types.Add(type);
    m_name.Add("en", "Sample");
    m_houseNumber = "12";
  }
};

class StyleSnapshotFixture
{
public:
  StyleSnapshotFixture() : m_originalStyle(GetStyleReader().GetCurrentStyle())
  {
    classificator::Load();
    // Loading is confined to this thread; switches only publish resident, immutable families.
    for (auto const style : kStyles)
      classificator::EnsureStyleLoaded(style);
  }

  ~StyleSnapshotFixture() { GetStyleReader().SetCurrentStyle(m_originalStyle); }

private:
  MapStyle const m_originalStyle;
};

void TestSameRules(df::Stylist const & actual, df::Stylist const & expected)
{
  TEST_EQUAL(actual.m_symbolRule, expected.m_symbolRule, ());
  TEST_EQUAL(actual.m_captionRule, expected.m_captionRule, ());
  TEST_EQUAL(actual.m_houseNumberRule, expected.m_houseNumberRule, ());
  TEST_EQUAL(actual.m_pathtextRule, expected.m_pathtextRule, ());
  TEST_EQUAL(actual.m_shieldRule, expected.m_shieldRule, ());
  TEST_EQUAL(actual.m_areaRule, expected.m_areaRule, ());
  TEST_EQUAL(actual.m_hatchingRule, expected.m_hatchingRule, ());
  TEST(std::equal(actual.m_lineRules.begin(), actual.m_lineRules.end(), expected.m_lineRules.begin(),
                  expected.m_lineRules.end()),
       ());
}

UNIT_CLASS_TEST(StyleSnapshotFixture, Stylist_ExplicitStyleIgnoresGlobalSwitches)
{
  using feature::GeomType;
  struct Sample
  {
    uint32_t m_type;
    GeomType m_geomType;
  };
  auto const & cl = classif(MapStyleDefaultLight);
  std::array const samples = {Sample{cl.GetTypeByPath({"highway", "motorway"}), GeomType::Line},
                              Sample{cl.GetTypeByPath({"highway", "path"}), GeomType::Line},
                              Sample{cl.GetTypeByPath({"amenity", "cafe"}), GeomType::Point},
                              Sample{cl.GetTypeByPath({"building"}), GeomType::Area},
                              Sample{cl.GetTypeByPath({"natural", "water", "lake"}), GeomType::Area}};

  for (auto const style : kStyles)
    for (auto const & sample : samples)
    {
      auto feature = FeatureType::CreateFromMapObject(SampleFeature(sample.m_type, sample.m_geomType));
      for (uint8_t zoom = 1; zoom <= 19; ++zoom)
      {
        GetStyleReader().SetCurrentStyle(style);
        df::Stylist const expected(*feature, zoom, StringUtf8Multilang::kEnglishCode, style);
        for (auto const current : kStyles)
        {
          GetStyleReader().SetCurrentStyle(current);
          df::Stylist const actual(*feature, zoom, StringUtf8Multilang::kEnglishCode, style);
          TestSameRules(actual, expected);
        }
      }
    }
}

UNIT_CLASS_TEST(StyleSnapshotFixture, Stylist_ExplicitStyleDuringConcurrentSwitches)
{
  auto const type = classif(MapStyleDefaultLight).GetTypeByPath({"highway", "primary"});
  auto feature = FeatureType::CreateFromMapObject(SampleFeature(type, feature::GeomType::Line));
  df::Stylist const expected(*feature, 16, StringUtf8Multilang::kEnglishCode, MapStyleVehicleLight);
  TEST(!expected.m_lineRules.empty(), ());

  std::latch started(1);
  std::jthread switcher([&](std::stop_token stop)
  {
    started.count_down();
    while (!stop.stop_requested())
      for (auto const style : kStyles)
        GetStyleReader().SetCurrentStyle(style);
  });
  started.wait();
  for (size_t i = 0; i < 10000; ++i)
  {
    df::Stylist const actual(*feature, 16, StringUtf8Multilang::kEnglishCode, MapStyleVehicleLight);
    TestSameRules(actual, expected);
  }
}
}  // namespace style_snapshot_tests
