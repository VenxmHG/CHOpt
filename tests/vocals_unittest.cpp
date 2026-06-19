#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <filesystem>

#include "imagebuilder.hpp"
#include "songfile.hpp"
#include "test_helpers.hpp"
#include "vocals.hpp"

namespace {
SightRead::VocalTrack
make_vocal_track(std::vector<SightRead::VocalPhrase> phrases,
                 std::vector<SightRead::VocalTube> tubes = {},
                 std::vector<SightRead::LyricEvent> lyrics = {})
{
    return {std::move(tubes), std::move(lyrics), std::move(phrases),
            std::make_shared<SightRead::SongGlobalData>()};
}

SightRead::VocalTrack
make_vocal_track_at_bpm(double bpm, std::vector<SightRead::VocalPhrase> phrases,
                        std::vector<SightRead::VocalTube> tubes = {},
                        std::vector<SightRead::LyricEvent> lyrics = {})
{
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(
        SightRead::TempoMap {{},
                             {{.position = SightRead::Tick {0},
                               .millibeats_per_minute = bpm * 1000.0}},
                             {},
                             192});
    return {std::move(tubes), std::move(lyrics), std::move(phrases),
            std::move(global_data)};
}

SightRead::Tick measure_tick(double measure)
{
    constexpr auto TICKS_PER_MEASURE = 768.0;
    return SightRead::Tick {
        static_cast<int>(std::lround(measure * TICKS_PER_MEASURE))};
}

SightRead::Tick beat_tick(double beat)
{
    constexpr auto TICKS_PER_BEAT = 192.0;
    return SightRead::Tick {
        static_cast<int>(std::lround(beat * TICKS_PER_BEAT))};
}
}

BOOST_AUTO_TEST_SUITE(vocals_processed_song)

BOOST_AUTO_TEST_CASE(phrase_multiplier_increases_and_caps_at_four_x)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {1152},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {1536},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {96},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {96},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {96},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {1152},
                             .length = SightRead::Tick {96},
                             .pitch = 65,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {1536},
                             .length = SightRead::Tick {96},
                             .pitch = 67,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.phrases().size(), 5U);
    BOOST_CHECK_EQUAL(song.phrases().at(0).multiplier, 1);
    BOOST_CHECK_EQUAL(song.phrases().at(1).multiplier, 2);
    BOOST_CHECK_EQUAL(song.phrases().at(2).multiplier, 3);
    BOOST_CHECK_EQUAL(song.phrases().at(3).multiplier, 4);
    BOOST_CHECK_EQUAL(song.phrases().at(4).multiplier, 4);
}

BOOST_AUTO_TEST_CASE(rb_phrase_base_scores_follow_combo_multiplier)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {1152},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {1536},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {96},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {96},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {96},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {1152},
                             .length = SightRead::Tick {96},
                             .pitch = 65,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {1536},
                             .length = SightRead::Tick {96},
                             .pitch = 67,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track,
                                    default_rb_vocals_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.phrases().size(), 5U);
    BOOST_CHECK_EQUAL(song.phrases().at(0).base_score, 1000);
    BOOST_CHECK_EQUAL(song.phrases().at(1).base_score, 2000);
    BOOST_CHECK_EQUAL(song.phrases().at(2).base_score, 3000);
    BOOST_CHECK_EQUAL(song.phrases().at(3).base_score, 4000);
    BOOST_CHECK_EQUAL(song.phrases().at(4).base_score, 4000);
    BOOST_CHECK_EQUAL(song.total_base_score(), 14000);
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_phrase_base_scores_ignore_tube_count)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {96},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {480},
                             .length = SightRead::Tick {96},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {64},
                             .pitch = 65,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {832},
                             .length = SightRead::Tick {64},
                             .pitch = 67,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {896},
                             .length = SightRead::Tick {64},
                             .pitch = 69,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.phrases().size(), 3U);
    BOOST_CHECK_EQUAL(song.phrases().at(0).base_score, 1000);
    BOOST_CHECK_EQUAL(song.phrases().at(1).base_score, 2000);
    BOOST_CHECK_EQUAL(song.phrases().at(2).base_score, 3000);
    BOOST_CHECK_EQUAL(song.total_base_score(), 6000);
}

BOOST_AUTO_TEST_CASE(longer_tubes_have_more_pie_units)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {96},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track,
                                    default_rb_vocals_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.phrases().size(), 2U);
    BOOST_CHECK_GT(song.phrases().at(1).pie.target_units,
                   song.phrases().at(0).pie.target_units);
}

BOOST_AUTO_TEST_CASE(more_tubes_with_same_total_duration_have_more_pie_units)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {96},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {480},
                             .length = SightRead::Tick {96},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track,
                                    default_rb_vocals_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.phrases().size(), 2U);
    BOOST_CHECK_CLOSE(song.phrases().at(0).pie.target_units, 1.25, 0.0001);
    BOOST_CHECK_CLOSE(song.phrases().at(1).pie.target_units, 1.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(perfect_vibrato_fills_pitched_tubes_faster)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track,
                                    default_rb_vocals_pathing_settings()};

    const auto& phrase = song.phrases().front();
    const auto first_half_end = SightRead::Beat {0.5};
    BOOST_CHECK_GT(
        phrase.pie.fill_between(phrase.start, first_half_end, true),
        phrase.pie.fill_between(phrase.start, first_half_end, false));
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_has_no_vibrato_speedup)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    const auto& phrase = song.phrases().front();
    const auto first_half_end = SightRead::Beat {0.5};
    BOOST_CHECK_CLOSE(
        phrase.pie.fill_between(phrase.start, first_half_end, true),
        phrase.pie.fill_between(phrase.start, first_half_end, false), 0.0001);
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_tubes_full_credit_at_eighty_percent)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    const auto& phrase = song.phrases().front();
    BOOST_CHECK_LT(
        phrase.pie.fill_between(phrase.start, SightRead::Beat {0.79}), 1.0);
    BOOST_CHECK_CLOSE(
        phrase.pie.fill_between(phrase.start, SightRead::Beat {0.8}), 1.0,
        0.0001);
    BOOST_CHECK_CLOSE(
        phrase.pie.fill_between(SightRead::Beat {0.8}, phrase.end), 0.25,
        0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_black_hole_sun_esp_phrase_pie_matches_video)
{
    const auto track = make_vocal_track(
        {{.position = beat_tick(0.0),
          .length = beat_tick(7.0),
          .is_sp_phrase = false}},
        {{.position = beat_tick(0.5),
          .length = beat_tick(0.6875),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(1.28125),
          .length = beat_tick(0.375),
          .pitch = 63,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(2.03125),
          .length = beat_tick(0.15625),
          .pitch = 63,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(2.28125),
          .length = beat_tick(0.5),
          .pitch = 65,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(4.34375),
          .length = beat_tick(0.15625),
          .pitch = 56,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(4.59375),
          .length = beat_tick(0.40625),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(5.21875),
          .length = beat_tick(0.46875),
          .pitch = 63,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(6.15625),
          .length = beat_tick(0.15625),
          .pitch = 63,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(6.40625),
          .length = beat_tick(0.34375),
          .pitch = 65,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    const auto& phrase = song.phrases().front();
    BOOST_CHECK_CLOSE(phrase.pie.target_units, 5.5, 0.0001);
    BOOST_CHECK_CLOSE(
        phrase.pie.fill_between(SightRead::Beat {0.0},
                                SightRead::Beat {2.0}),
        0.3551136364, 0.0001);
    BOOST_CHECK_CLOSE(
        phrase.pie.fill_between(SightRead::Beat {2.0},
                                SightRead::Beat {6.0}),
        0.6676136364, 0.0001);
    BOOST_CHECK_CLOSE(
        phrase.pie.fill_between(SightRead::Beat {6.0},
                                SightRead::Beat {7.0}),
        0.2272727273, 0.0001);

    const auto esp_start = SightRead::Beat {2.68125};
    BOOST_CHECK_CLOSE(phrase.pie.required_prefill(esp_start), 0.3338068182,
                      0.0001);
    BOOST_CHECK_CLOSE(phrase.pie.boosted_fraction(esp_start, phrase.end),
                      0.6661931818, 0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_all_eyz_on_me_phrase_can_recover_from_missed_start)
{
    const auto track = make_vocal_track(
        {{.position = beat_tick(0.0),
          .length = beat_tick(5.25),
          .is_sp_phrase = false}},
        {{.position = beat_tick(0.25),
          .length = beat_tick(0.125),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(0.53125),
          .length = beat_tick(0.1875),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(1.78125),
          .length = beat_tick(0.15625),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(2.28125),
          .length = beat_tick(0.125),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(2.53125),
          .length = beat_tick(0.125),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(2.78125),
          .length = beat_tick(0.3125),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(3.34375),
          .length = beat_tick(0.34375),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(4.0),
          .length = beat_tick(0.09375),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(4.3125),
          .length = beat_tick(0.0625),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(4.5),
          .length = beat_tick(0.09375),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(4.78125),
          .length = beat_tick(0.21875),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    const auto& phrase = song.phrases().front();
    BOOST_CHECK_CLOSE(phrase.pie.target_units, 4.59375, 0.0001);
    BOOST_CHECK_CLOSE(
        phrase.pie.required_prefill(SightRead::Beat {1.78125}), 0.0,
        0.0001);
    BOOST_CHECK_CLOSE(
        phrase.pie.fill_between(SightRead::Beat {1.78125}, phrase.end), 1.0,
        0.0001);
    BOOST_CHECK_CLOSE(
        phrase.pie.required_prefill(SightRead::Beat {2.28125}), 0.0816326531,
        0.0001);
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_coral_final_phrase_reaches_full_pie)
{
    const auto track = make_vocal_track(
        {{.position = beat_tick(0.0),
          .length = beat_tick(8.5),
          .is_sp_phrase = false}},
        {{.position = beat_tick(0.5),
          .length = beat_tick(0.1875),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(0.875),
          .length = beat_tick(2.0),
          .pitch = 62,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(3.0),
          .length = beat_tick(0.5),
          .pitch = 64,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(3.6875),
          .length = beat_tick(0.6875),
          .pitch = 65,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(4.5),
          .length = beat_tick(0.875),
          .pitch = 67,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(5.5),
          .length = beat_tick(0.375),
          .pitch = 69,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(6.0625),
          .length = beat_tick(1.9375),
          .pitch = 71,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    const auto& phrase = song.phrases().front();
    const auto coral_activation_end = SightRead::Beat {7.6384615385};
    BOOST_CHECK_CLOSE(
        phrase.pie.fill_between(phrase.start, coral_activation_end), 1.0,
        0.0001);
}

BOOST_AUTO_TEST_CASE(activation_windows_are_generated_from_phrase_content_gaps)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {96},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {420},
                             .length = SightRead::Tick {60},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {192},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Unpitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 2U);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).start.value(), 0.5,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(), 2.1875,
                      0.0001);
    BOOST_CHECK_EQUAL(song.activation_windows().at(0).target_phrase_index, 1U);
    BOOST_CHECK_CLOSE(song.activation_windows().at(1).start.value(), 2.5,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(1).end.value(), 4.0, 0.0001);
    BOOST_CHECK_EQUAL(song.activation_windows().at(1).target_phrase_index, 2U);
}

BOOST_AUTO_TEST_CASE(short_inter_phrase_gaps_do_not_create_activation_windows)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {240},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {240},
                             .length = SightRead::Tick {192},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_CHECK(song.activation_windows().empty());
}

BOOST_AUTO_TEST_CASE(vocal_path_summary_reports_skipped_activation_windows)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {96},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {420},
                             .length = SightRead::Tick {60},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {192},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Unpitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalPath path {
        .activations = {{.start_phrase_index = 2,
                         .end_phrase_index = 2,
                         .start = song.activation_windows().at(1).start,
                         .end = song.phrases().at(2).end,
                         .sp_start = 0.25}},
        .phrase_score_boosts = {0, 0, song.phrases().at(2).base_score},
        .score_boost = song.phrases().at(2).base_score};

    BOOST_CHECK_EQUAL(song.path_summary(path), "Acts: 1/s1");
    BOOST_CHECK_EQUAL(song.path_summary(path, VocalPathNotation::ScoreHero),
                      "Acts: 1/sk1");
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_formats_compact_esf_esp_percentages)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalActivation esf_activation {.start_phrase_index = 0,
                                          .end_phrase_index = 0,
                                          .start = SightRead::Beat {0.5},
                                          .end = SightRead::Beat {1.0},
                                          .sp_start = 0.25,
                                          .esf_annotation = "S"};
    const VocalActivation esp_activation {.start_phrase_index = 0,
                                          .end_phrase_index = 0,
                                          .start = SightRead::Beat {0.5},
                                          .end = SightRead::Beat {1.0},
                                          .sp_start = 0.25,
                                          .esf_annotation = "S60"};

    BOOST_CHECK_EQUAL(song.squeeze_text(esf_activation), "S0");
    BOOST_CHECK_EQUAL(song.squeeze_text(esf_activation,
                                        VocalPathNotation::ScoreHero),
                      "ESF0");
    BOOST_CHECK_EQUAL(song.squeeze_text(esp_activation), "S60");
    BOOST_CHECK_EQUAL(song.squeeze_text(esp_activation,
                                        VocalPathNotation::ScoreHero),
                      "ESP60");
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_no_sing_guides_only_cover_tubes)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {576},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    ImageBuilder builder {track, SightRead::Difficulty::Expert, false};
    const VocalPath path {
        .activations = {{.start_phrase_index = 0,
                         .end_phrase_index = 0,
                         .start = SightRead::Beat {2.0},
                         .end = SightRead::Beat {3.0},
                         .sp_start = 0.25,
                         .required_prefill_fraction = 0.5,
                         .esf_annotation = "S50"}}};

    builder.add_vocal_squeeze_guides(song, path,
                                     VocalPathNotation::ScoreHero);

    BOOST_REQUIRE_EQUAL(builder.vocal_no_sing_ranges().size(), 1U);
    BOOST_CHECK_CLOSE(builder.vocal_no_sing_ranges().front().start, 0.8,
                      0.001);
    BOOST_CHECK_CLOSE(builder.vocal_no_sing_ranges().front().end, 1.0,
                      0.0001);
    BOOST_REQUIRE_EQUAL(builder.vocal_squeeze_labels().size(), 1U);
    BOOST_CHECK_EQUAL(builder.vocal_squeeze_labels().front().text, "ESP50");
}

BOOST_AUTO_TEST_CASE(vocal_path_summary_counts_skips_after_mid_phrase_sp_pickup)
{
    const auto track = make_vocal_track_at_bpm(
        120.0,
        {{.position = measure_tick(0.0),
          .length = measure_tick(0.5),
          .is_sp_phrase = true},
         {.position = measure_tick(1.0),
          .length = measure_tick(1.0),
          .is_sp_phrase = true},
         {.position = measure_tick(2.25),
          .length = measure_tick(0.75),
          .is_sp_phrase = false},
         {.position = measure_tick(4.0),
          .length = measure_tick(0.75),
          .is_sp_phrase = false},
         {.position = measure_tick(5.75),
          .length = measure_tick(0.75),
          .is_sp_phrase = false}},
        {{.position = measure_tick(0.0),
          .length = measure_tick(0.25),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(1.0),
          .length = measure_tick(0.5),
          .pitch = 62,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(2.75),
          .length = measure_tick(0.25),
          .pitch = 64,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(4.5),
          .length = measure_tick(0.25),
          .pitch = 65,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(6.25),
          .length = measure_tick(0.25),
          .pitch = 67,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalPath path {
        .activations = {{.start_phrase_index = 0,
                         .end_phrase_index = 1,
                         .start = song.phrases().at(0).start,
                         .end = song.tempo_map().to_beats(measure_tick(1.5)),
                         .sp_start = 0.25},
                        {.start_phrase_index = 4,
                         .end_phrase_index = 4,
                         .start = song.activation_windows().at(3).start,
                         .end = song.phrases().at(4).end,
                         .sp_start = 0.25}},
        .phrase_score_boosts = {0, 0, 0, 0, song.phrases().at(4).base_score},
        .score_boost = song.phrases().at(4).base_score};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 4U);
    BOOST_CHECK_LT(path.activations.at(0).end.value(),
                   song.phrases().at(1).end.value());
    BOOST_CHECK_EQUAL(song.path_summary(path), "Acts: 1/ 1/s2");
    BOOST_CHECK_EQUAL(song.path_summary(path, VocalPathNotation::ScoreHero),
                      "Acts: 1/ 1/sk2");
}

BOOST_AUTO_TEST_CASE(
    vocal_path_summary_counts_sp_phrases_after_the_previous_activation)
{
    const auto track = make_vocal_track({{.position = SightRead::Tick {0},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {192},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {384},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = false},
                                         {.position = SightRead::Tick {576},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {768},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {960},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {1152},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {1344},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = false}});
    const VocalsProcessedSong song {track,
                                    default_rb_vocals_pathing_settings()};
    const VocalPath path {.activations = {{.start_phrase_index = 2,
                                           .end_phrase_index = 3,
                                           .start = song.phrases().at(2).start,
                                           .end = song.phrases().at(3).end,
                                           .sp_start = 0.5},
                                          {.start_phrase_index = 7,
                                           .end_phrase_index = 7,
                                           .start = song.phrases().at(7).start,
                                           .end = song.phrases().at(7).end,
                                           .sp_start = 0.75}},
                          .phrase_score_boosts = {0, 0, 0, 0, 0, 0, 0, 0},
                          .score_boost = 0};

    BOOST_CHECK_EQUAL(song.path_summary(path), "Acts: 2/ 3/");
    BOOST_CHECK_EQUAL(song.path_summary(path, VocalPathNotation::ScoreHero),
                      "Acts: 2/ 3/");
}

BOOST_AUTO_TEST_CASE(vocal_path_summary_uses_sp_available_at_activation_start)
{
    const auto track = make_vocal_track({{.position = SightRead::Tick {0},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {192},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {384},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = false},
                                         {.position = SightRead::Tick {576},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {768},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = true},
                                         {.position = SightRead::Tick {960},
                                          .length = SightRead::Tick {192},
                                          .is_sp_phrase = false}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalPath path {.activations = {{.start_phrase_index = 2,
                                           .end_phrase_index = 3,
                                           .start = song.phrases().at(2).start,
                                           .end = song.phrases().at(3).end,
                                           .sp_start = 0.5},
                                          {.start_phrase_index = 5,
                                           .end_phrase_index = 5,
                                           .start = song.phrases().at(5).start,
                                           .end = song.phrases().at(5).end,
                                           .sp_start = 0.5}},
                          .phrase_score_boosts = {0, 0, 0, 0, 0, 0},
                          .score_boost = 0};

    BOOST_CHECK_EQUAL(song.path_summary(path), "Acts: 2/ 2/");
    BOOST_CHECK_EQUAL(song.path_summary(path, VocalPathNotation::ScoreHero),
                      "Acts: 2/ 2/");
}

BOOST_AUTO_TEST_CASE(vocal_path_summary_reports_before_od_activations)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {1152},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {96},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {420},
                             .length = SightRead::Tick {96},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {804},
                             .length = SightRead::Tick {96},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {1200},
                             .length = SightRead::Tick {96},
                             .pitch = 65,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const auto before_od_window = std::ranges::find_if(
        song.activation_windows(),
        [&](const auto& window) { return window.target_phrase_index == 3U; });
    BOOST_REQUIRE(before_od_window != song.activation_windows().cend());
    const VocalPath path {.activations = {{.start_phrase_index = 3,
                                           .end_phrase_index = 3,
                                           .start = before_od_window->start,
                                           .end = song.phrases().at(3).end,
                                           .sp_start = 0.25}},
                          .phrase_score_boosts = {0, 0, 0, 0},
                          .score_boost = 0};

    BOOST_CHECK_LT(path.activations.at(0).start.value(),
                   song.phrases().at(3).start.value());
    BOOST_CHECK_EQUAL(song.path_summary(path), "Acts: 1/B");
    BOOST_CHECK_EQUAL(song.path_summary(path, VocalPathNotation::ScoreHero),
                      "Acts: 1/BOD");
}

BOOST_AUTO_TEST_CASE(
    multiple_internal_phrase_gaps_create_multiple_windows_for_one_phrase)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {1104},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {432},
                             .length = SightRead::Tick {192},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {864},
                             .length = SightRead::Tick {192},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {1296},
                             .length = SightRead::Tick {192},
                             .pitch = 65,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 3U);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).start.value(), 1.0,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(), 2.25,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(1).start.value(), 3.25,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(1).end.value(), 4.5, 0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(2).start.value(), 5.5,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(2).end.value(), 6.75,
                      0.0001);
    BOOST_CHECK_EQUAL(song.activation_windows().at(0).target_phrase_index, 1U);
    BOOST_CHECK_EQUAL(song.activation_windows().at(1).target_phrase_index, 1U);
    BOOST_CHECK_EQUAL(song.activation_windows().at(2).target_phrase_index, 1U);
}

BOOST_AUTO_TEST_CASE(one_sp_phrase_is_enough_to_start_an_activation)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {96},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {96},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {48},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_CHECK_EQUAL(path.activations.at(0).start_phrase_index, 1U);
    BOOST_CHECK_EQUAL(path.activations.at(0).end_phrase_index, 1U);
    BOOST_CHECK_GT(path.activations.at(0).start.value(), 1.99);
    BOOST_CHECK_LT(path.activations.at(0).start.value(), 2.0);
    BOOST_CHECK_GE(path.activations.at(0).sp_start, 0.25);
    BOOST_CHECK_GT(path.score_boost, 0);
    BOOST_CHECK_EQUAL(song.path_summary(path), "Acts: 1/");
}

BOOST_AUTO_TEST_CASE(
    karaoke_first_usable_sp_has_readiness_delay_but_skips_window)
{
    const auto track
        = make_vocal_track_at_bpm(
            120.0,
            {{.position = beat_tick(0.0),
              .length = beat_tick(4.0),
              .is_sp_phrase = true},
             {.position = beat_tick(4.0),
              .length = beat_tick(4.0),
              .is_sp_phrase = false},
             {.position = beat_tick(8.0),
              .length = beat_tick(3.0),
              .is_sp_phrase = false}},
            {{.position = beat_tick(0.0),
              .length = beat_tick(2.75),
              .pitch = 60,
              .type = SightRead::VocalTubeType::Pitched},
             {.position = beat_tick(4.75),
              .length = beat_tick(0.5),
              .pitch = 62,
              .type = SightRead::VocalTubeType::Pitched},
             {.position = beat_tick(9.5),
              .length = beat_tick(1.0),
              .pitch = 64,
              .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 2U);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).start.value(), 2.75,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(), 4.75,
                      0.0001);

    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_CHECK_EQUAL(path.activations.at(0).start_phrase_index, 2U);
    BOOST_CHECK_GT(path.activations.at(0).start.value(), 9.49);
    BOOST_CHECK_EQUAL(song.path_summary(path, VocalPathNotation::ScoreHero),
                      "Acts: 1/sk1");
    BOOST_CHECK_GT(path.score_boost, 0);
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_squeeze_limits_late_activation_starts)
{
    auto settings = default_karaoke_pathing_settings();
    settings.squeeze = 0.5;
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {768},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {384},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, settings};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).start.value(), 1.0,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(), 4.0, 0.0001);

    const auto& starts = song.activation_starts(1);
    BOOST_REQUIRE_EQUAL(starts.size(), 3U);
    BOOST_CHECK_CLOSE(starts.at(0).value(), 1.0, 0.0001);
    BOOST_CHECK_CLOSE(starts.at(1).value(), 2.0, 0.0001);
    BOOST_CHECK_CLOSE(starts.at(2).value(), 2.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(
    activation_before_first_tube_after_phrase_start_is_not_an_esf)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {384},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .pitch = 61,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {816},
                             .length = SightRead::Tick {336},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track,
                                    default_rb_vocals_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_CHECK_GT(path.activations.at(0).start.value(),
                   song.phrases().at(2).start.value());
    BOOST_CHECK_LT(path.activations.at(0).start.value(),
                   song.phrases().at(2).scored_segments.front().start.value());
    BOOST_CHECK(path.activations.at(0).esf_annotation.empty());
    BOOST_CHECK_EQUAL(song.path_summary(path), "Acts: 2/");
    BOOST_CHECK_EQUAL(song.path_summary(path, VocalPathNotation::ScoreHero),
                      "Acts: 2/");
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_esf_prefill_uses_tube_credit_without_vibrato)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {768},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {1},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {384},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_CHECK_EQUAL(path.activations.at(0).esf_annotation, "S");
    BOOST_CHECK_CLOSE(path.activations.at(0).boosted_pie_fraction,
                      1.0, 0.0001);
    BOOST_CHECK_CLOSE(path.activations.at(0).required_prefill_fraction,
                      0.0, 0.0001);
    BOOST_REQUIRE_EQUAL(path.phrase_score_boosts.size(), 2U);
    BOOST_CHECK_EQUAL(path.phrase_score_boosts.at(1), 2000);
    BOOST_CHECK_EQUAL(song.path_summary(path), "Acts: 1/S0");
    BOOST_CHECK_EQUAL(song.path_summary(path, VocalPathNotation::ScoreHero),
                      "Acts: 1/ESF0");
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_mid_phrase_activation_does_not_refill_active_sp_phrase)
{
    const auto track = make_vocal_track_at_bpm(
        300.0,
        {{.position = beat_tick(0.0),
          .length = beat_tick(4.0),
          .is_sp_phrase = true},
         {.position = beat_tick(4.0),
          .length = beat_tick(8.0),
          .is_sp_phrase = true},
         {.position = beat_tick(30.5),
          .length = beat_tick(2.0),
          .is_sp_phrase = false}},
        {{.position = beat_tick(0.0),
          .length = beat_tick(4.0),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(4.0),
          .length = beat_tick(0.1),
          .pitch = 62,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(10.0),
          .length = beat_tick(2.0),
          .pitch = 64,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = beat_tick(30.5),
          .length = beat_tick(2.0),
          .pitch = 65,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE(song.active_sp_pickup_requires_phrase_start());
    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_CHECK_GT(path.activations.at(0).start.value(),
                   song.phrases().at(1).start.value());
    BOOST_CHECK_EQUAL(path.activations.at(0).end_phrase_index, 1U);
    BOOST_CHECK_LT(path.activations.at(0).end.value(),
                   song.phrases().at(2).start.value());
    BOOST_REQUIRE_EQUAL(path.phrase_score_boosts.size(), 3U);
    BOOST_CHECK_EQUAL(path.phrase_score_boosts.at(2), 0);
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_uses_tuned_od_drain)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong karaoke_song {track,
                                            default_karaoke_pathing_settings()};
    const VocalsProcessedSong rb_song {track,
                                       default_rb_vocals_pathing_settings()};

    BOOST_CHECK_CLOSE(karaoke_song.sp_drain_rate(), 0.99824, 0.0001);
    BOOST_CHECK_CLOSE(rb_song.sp_drain_rate(), 1.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_like_jennie_matches_known_optimal)
{
    const std::filesystem::path midi_path {"tests/like JENNIE.mid"};
    if (!std::filesystem::exists(midi_path)) {
        BOOST_TEST_MESSAGE(
            "Skipping like JENNIE regression; test MIDI is not present");
        return;
    }

    const SongFile song_file {midi_path.string()};
    const auto song = song_file.load_song(Game::FortniteFestival);
    const auto& track = song.vocal_track(SightRead::Instrument::Vocals,
                                         SightRead::Difficulty::Expert);
    const VocalsProcessedSong processed {track,
                                         default_karaoke_pathing_settings()};
    const VocalsOptimiser optimiser {&processed};
    const auto path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(processed.path_summary(path,
                                             VocalPathNotation::ScoreHero),
                      "Acts: 4/sk2 1/ 2/");
    BOOST_CHECK_EQUAL(processed.total_base_score() + path.score_boost,
                      237723);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_quarter_bar_act_reaches_all_eyez_chain_distance)
{
    constexpr auto ALL_EYEZ_CHAIN_DISTANCE_MEASURES = 1.9296875;
    constexpr auto ACTIVATION_START_BEAT = 8.0;
    constexpr auto SP_PHRASE_END_BEAT
        = ACTIVATION_START_BEAT + ALL_EYEZ_CHAIN_DISTANCE_MEASURES * 4.0;
    const auto track
        = make_vocal_track({{.position = beat_tick(0.0),
                             .length = beat_tick(1.0),
                             .is_sp_phrase = true},
                            {.position = beat_tick(4.0),
                             .length = beat_tick(4.5),
                             .is_sp_phrase = false},
                            {.position = beat_tick(8.5),
                             .length = beat_tick(SP_PHRASE_END_BEAT - 8.5),
                             .is_sp_phrase = true}},
                           {{.position = beat_tick(0.0),
                             .length = beat_tick(1.0),
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = beat_tick(ACTIVATION_START_BEAT),
                             .length = beat_tick(0.5),
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = beat_tick(8.5),
                             .length = beat_tick(SP_PHRASE_END_BEAT - 8.5),
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_CHECK_CLOSE(path.activations.front().start.value(),
                      ACTIVATION_START_BEAT, 0.0001);
    BOOST_CHECK_GT(path.activations.front().end.value(),
                   song.phrases().at(2).start.value());
    BOOST_CHECK_CLOSE(path.activations.front().end.value(),
                      song.phrases().at(2).end.value(), 0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_prefill_esf_reports_required_prefill_and_partial_boost)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {240},
                             .length = SightRead::Tick {1152},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {240},
                             .length = SightRead::Tick {714},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {1200},
                             .length = SightRead::Tick {192},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_CHECK_EQUAL(path.activations.at(0).esf_annotation, "S71");
    BOOST_CHECK_CLOSE(path.activations.at(0).boosted_pie_fraction,
                      0.2994011976, 0.0001);
    BOOST_CHECK_CLOSE(path.activations.at(0).required_prefill_fraction,
                      0.7005988024,
                      0.0001);
    BOOST_REQUIRE_EQUAL(path.phrase_score_boosts.size(), 2U);
    BOOST_CHECK_EQUAL(path.phrase_score_boosts.at(1), 599);
    BOOST_CHECK_EQUAL(song.path_summary(path), "Acts: 1/S71");
    BOOST_CHECK_EQUAL(song.path_summary(path, VocalPathNotation::ScoreHero),
                      "Acts: 1/ESP71");
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_activation_ending_mid_phrase_only_boosts_active_pie_fill)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {4608},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {1},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {3840},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_REQUIRE_EQUAL(path.phrase_score_boosts.size(), 2U);
    BOOST_CHECK_LT(path.activations.at(0).boosted_pie_fraction, 1.0);
    BOOST_CHECK_GT(path.activations.at(0).boosted_pie_fraction, 0.0);
    BOOST_CHECK_EQUAL(path.phrase_score_boosts.at(1),
                      static_cast<int>(std::lround(
                          song.phrases().at(1).base_score
                          * path.activations.at(0).boosted_pie_fraction)));
}

BOOST_AUTO_TEST_CASE(
    later_internal_windows_allow_partial_phrase_boosts_and_future_carry)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {360},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {624},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {1008},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {816},
                             .length = SightRead::Tick {192},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {1008},
                             .length = SightRead::Tick {192},
                             .pitch = 65,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_CHECK_EQUAL(path.activations.at(0).start_phrase_index, 1U);
    BOOST_CHECK_EQUAL(path.activations.at(0).end_phrase_index, 2U);
    BOOST_CHECK_GT(path.activations.at(0).start.value(), 4.24);
    BOOST_CHECK_LT(path.activations.at(0).start.value(), 4.25);
    BOOST_CHECK_CLOSE(path.activations.at(0).end.value(), 6.25, 0.0001);

    BOOST_REQUIRE_EQUAL(path.phrase_score_boosts.size(), 3U);
    BOOST_CHECK_EQUAL(path.phrase_score_boosts.at(0), 0);
    BOOST_CHECK_GT(path.activations.at(0).boosted_pie_fraction, 0.0);
    BOOST_CHECK_LT(path.activations.at(0).boosted_pie_fraction, 1.0);
    const auto first_boost = static_cast<int>(
        std::lround(song.phrases().at(1).base_score
                    * path.activations.at(0).boosted_pie_fraction));
    BOOST_CHECK_EQUAL(path.phrase_score_boosts.at(1), first_boost);
    BOOST_CHECK_EQUAL(path.phrase_score_boosts.at(2),
                      song.phrases().at(2).base_score);
    BOOST_CHECK_EQUAL(path.score_boost,
                      first_boost + song.phrases().at(2).base_score);
}

BOOST_AUTO_TEST_CASE(small_internal_phrase_gaps_do_not_create_windows)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {632},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {392},
                             .length = SightRead::Tick {240},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_CHECK(song.activation_windows().empty());
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_phrase_boundary_gaps_use_full_tube_to_tube_duration)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {240},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {240},
                             .length = SightRead::Tick {384},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {432},
                             .length = SightRead::Tick {192},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_EQUAL(song.activation_windows().at(0).target_phrase_index, 1U);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).start.value(), 1.0,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(), 2.25,
                      0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_shows_three_and_half_measure_post_phrase_windows)
{
    const auto track
        = make_vocal_track({{.position = measure_tick(0.0),
                             .length = measure_tick(1.0),
                             .is_sp_phrase = false},
                            {.position = measure_tick(4.5),
                             .length = measure_tick(1.0),
                             .is_sp_phrase = false}},
                           {{.position = measure_tick(0.0),
                             .length = measure_tick(1.0),
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(4.5),
                             .length = measure_tick(0.5),
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_EQUAL(song.activation_windows().at(0).target_phrase_index, 1U);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).start.value(), 4.0,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(), 18.0,
                      0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_large_post_phrase_gaps_reappear_before_next_phrase)
{
    const auto track = make_vocal_track_at_bpm(
        97.0,
        {{.position = measure_tick(42.0),
          .length = measure_tick(1.0),
          .is_sp_phrase = false},
         {.position = measure_tick(46.875),
          .length = measure_tick(2.0),
          .is_sp_phrase = false}},
        {{.position = measure_tick(42.0),
          .length = measure_tick(1.0),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(47.0),
          .length = measure_tick(0.5),
          .pitch = 62,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_CLOSE(song.tempo_map()
                          .to_measures(song.activation_windows().at(0).start)
                          .value(),
                      46.5, 0.01);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(),
                      measure_tick(47.0).value() / 192.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_post_phrase_reappear_allows_short_game_window)
{
    const auto track = make_vocal_track_at_bpm(
        171.0,
        {{.position = measure_tick(117.625),
          .length = measure_tick(1.5),
          .is_sp_phrase = false},
         {.position = measure_tick(122.875),
          .length = measure_tick(2.8125),
          .is_sp_phrase = false}},
        {{.position = measure_tick(117.734375),
          .length = measure_tick(1.234375),
          .pitch = 78,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(123.0),
          .length = measure_tick(0.0546875),
          .pitch = 62,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_CLOSE(song.tempo_map()
                          .to_measures(song.activation_windows().at(0).start)
                          .value(),
                      122.625, 0.01);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(),
                      measure_tick(123.0).value() / 192.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_long_post_phrase_gaps_reappear_near_next_phrase)
{
    const auto track = make_vocal_track_at_bpm(
        97.0,
        {{.position = measure_tick(0.0),
          .length = measure_tick(1.0),
          .is_sp_phrase = false},
         {.position = measure_tick(20.75),
          .length = measure_tick(1.0),
          .is_sp_phrase = false}},
        {{.position = measure_tick(0.0),
          .length = measure_tick(1.0),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(20.75),
          .length = measure_tick(0.5),
          .pitch = 62,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_EQUAL(song.activation_windows().at(0).target_phrase_index, 1U);
    BOOST_CHECK_CLOSE(song.tempo_map()
                          .to_measures(song.activation_windows().at(0).start)
                          .value(),
                      19.6875, 0.01);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(),
                      measure_tick(20.75).value() / 192.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_very_fast_long_post_phrase_gaps_can_stay_hidden)
{
    const auto track = make_vocal_track_at_bpm(
        300.0,
        {{.position = measure_tick(0.0),
          .length = measure_tick(1.0),
          .is_sp_phrase = false},
         {.position = measure_tick(20.75),
          .length = measure_tick(1.0),
          .is_sp_phrase = false}},
        {{.position = measure_tick(0.0),
          .length = measure_tick(1.0),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(20.75),
          .length = measure_tick(0.5),
          .pitch = 62,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_CHECK(song.activation_windows().empty());
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_faster_long_post_phrase_gaps_reappear_window_shrinks)
{
    const auto track = make_vocal_track_at_bpm(
        120.0,
        {{.position = measure_tick(0.0),
          .length = measure_tick(1.0),
          .is_sp_phrase = false},
         {.position = measure_tick(20.75),
          .length = measure_tick(1.0),
          .is_sp_phrase = false}},
        {{.position = measure_tick(0.0),
          .length = measure_tick(1.0),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(20.75),
          .length = measure_tick(0.5),
          .pitch = 62,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_CLOSE(song.tempo_map()
                          .to_measures(song.activation_windows().at(0).start)
                          .value(),
                      19.8911, 0.01);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(),
                      measure_tick(20.75).value() / 192.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_slower_long_post_phrase_gaps_reappear_window_grows)
{
    const auto track = make_vocal_track_at_bpm(
        80.0,
        {{.position = measure_tick(0.0),
          .length = measure_tick(1.0),
          .is_sp_phrase = false},
         {.position = measure_tick(20.75),
          .length = measure_tick(1.0),
          .is_sp_phrase = false}},
        {{.position = measure_tick(0.0),
          .length = measure_tick(1.0),
          .pitch = 60,
          .type = SightRead::VocalTubeType::Pitched},
         {.position = measure_tick(20.75),
          .length = measure_tick(0.5),
          .pitch = 62,
          .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_CLOSE(song.tempo_map()
                          .to_measures(song.activation_windows().at(0).start)
                          .value(),
                      19.4617, 0.01);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(),
                      measure_tick(20.75).value() / 192.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(
    fortnite_karaoke_uses_phrase_start_for_post_phrase_window_threshold)
{
    const auto track
        = make_vocal_track({{.position = measure_tick(0.0),
                             .length = measure_tick(1.0),
                             .is_sp_phrase = false},
                            {.position = measure_tick(4.25),
                             .length = measure_tick(1.0),
                             .is_sp_phrase = false}},
                           {{.position = measure_tick(0.0),
                             .length = measure_tick(1.0),
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(4.75),
                             .length = measure_tick(0.5),
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_EQUAL(song.activation_windows().at(0).target_phrase_index, 1U);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).start.value(), 4.0,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(), 19.0,
                      0.0001);
}

BOOST_AUTO_TEST_CASE(large_internal_phrase_gaps_create_windows)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {960},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {240},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {480},
                             .length = SightRead::Tick {480},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track, default_karaoke_pathing_settings()};

    BOOST_REQUIRE_EQUAL(song.activation_windows().size(), 1U);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).start.value(), 1.25,
                      0.0001);
    BOOST_CHECK_CLOSE(song.activation_windows().at(0).end.value(), 2.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(rb_vocals_require_two_sp_phrases_to_activate)
{
    const auto track
        = make_vocal_track({{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = true},
                            {.position = SightRead::Tick {1152},
                             .length = SightRead::Tick {192},
                             .is_sp_phrase = false}},
                           {{.position = SightRead::Tick {0},
                             .length = SightRead::Tick {96},
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {384},
                             .length = SightRead::Tick {96},
                             .pitch = 62,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {768},
                             .length = SightRead::Tick {96},
                             .pitch = 64,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = SightRead::Tick {1152},
                             .length = SightRead::Tick {96},
                             .pitch = 65,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track,
                                    default_rb_vocals_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_EQUAL(path.activations.size(), 1U);
    BOOST_CHECK_EQUAL(path.activations.at(0).start_phrase_index, 3U);
    BOOST_CHECK_GE(path.activations.at(0).sp_start, 0.5);
}

BOOST_AUTO_TEST_CASE(rb_vocals_late_esf_can_full_boost_two_phrases)
{
    const auto track
        = make_vocal_track({{.position = measure_tick(0.0),
                             .length = measure_tick(0.5),
                             .is_sp_phrase = true},
                            {.position = measure_tick(1.0),
                             .length = measure_tick(0.5),
                             .is_sp_phrase = true},
                            {.position = measure_tick(4.0),
                             .length = measure_tick(2.625),
                             .is_sp_phrase = false},
                            {.position = measure_tick(7.75),
                             .length = measure_tick(2.25),
                             .is_sp_phrase = true},
                            {.position = measure_tick(12.0),
                             .length = measure_tick(1.0),
                             .is_sp_phrase = true},
                            {.position = measure_tick(16.5),
                             .length = measure_tick(4.0),
                             .is_sp_phrase = false},
                            {.position = measure_tick(20.625),
                             .length = measure_tick(1.5),
                             .is_sp_phrase = false}},
                           {{.position = measure_tick(0.0),
                             .length = measure_tick(0.25),
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(1.0),
                             .length = measure_tick(0.25),
                             .pitch = 60,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(4.1),
                             .length = measure_tick(2.3),
                             .pitch = 68,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(7.9),
                             .length = measure_tick(1.6),
                             .pitch = 68,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(12.1),
                             .length = measure_tick(0.7),
                             .pitch = 68,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(16.679),
                             .length = measure_tick(0.259),
                             .pitch = 73,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(16.985),
                             .length = measure_tick(0.653),
                             .pitch = 61,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(18.670),
                             .length = measure_tick(0.221),
                             .pitch = 73,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(18.963),
                             .length = measure_tick(1.218),
                             .pitch = 73,
                             .type = SightRead::VocalTubeType::Pitched},
                            {.position = measure_tick(20.677),
                             .length = measure_tick(0.941),
                             .pitch = 73,
                             .type = SightRead::VocalTubeType::Pitched}});
    const VocalsProcessedSong song {track,
                                    default_rb_vocals_pathing_settings()};
    const VocalsOptimiser optimiser {&song};
    const auto path = optimiser.optimal_path();

    BOOST_REQUIRE_GE(path.activations.size(), 2U);
    const auto& late_activation = path.activations.back();
    BOOST_CHECK_EQUAL(late_activation.start_phrase_index, 5U);
    BOOST_CHECK_EQUAL(late_activation.end_phrase_index, 6U);
    BOOST_CHECK_CLOSE(
        song.tempo_map().to_measures(late_activation.start).value(), 18.67,
        0.01);
    BOOST_CHECK_EQUAL(late_activation.esf_annotation, "S");
    BOOST_REQUIRE_EQUAL(path.phrase_score_boosts.size(), 7U);
    BOOST_CHECK_EQUAL(path.phrase_score_boosts.at(5),
                      song.phrases().at(5).base_score);
    BOOST_CHECK_EQUAL(path.phrase_score_boosts.at(6),
                      song.phrases().at(6).base_score);
}

BOOST_AUTO_TEST_CASE(fortnite_karaoke_still_into_you_matches_phrase_start_path)
{
    const std::filesystem::path midi_path {"tests/Still Into You.mid"};
    if (!std::filesystem::exists(midi_path)) {
        BOOST_TEST_MESSAGE(
            "Skipping Still Into You regression; test MIDI is not present");
        return;
    }

    const SongFile song_file {midi_path.string()};
    const auto song = song_file.load_song(Game::FortniteFestival);
    const auto& track = song.vocal_track(SightRead::Instrument::Vocals,
                                         SightRead::Difficulty::Expert);
    const VocalsProcessedSong processed {track,
                                         default_karaoke_pathing_settings()};
    const VocalsOptimiser optimiser {&processed};
    const auto path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(processed.path_summary(path,
                                             VocalPathNotation::ScoreHero),
                      "Acts: 1/sk1 1/sk1 1/sk2 1/sk1 1/sk1 1/ 1/ 1/sk2");

    const std::vector<double> expected_start_measures {
        17.125, 33.125, 47.125, 55.367, 71.125, 86.117, 95.398, 114.836};
    BOOST_REQUIRE_EQUAL(path.activations.size(),
                        expected_start_measures.size());
    for (std::size_t i = 0; i < expected_start_measures.size(); ++i) {
        BOOST_CHECK_CLOSE(
            processed.tempo_map().to_measures(path.activations.at(i).start)
                    .value()
                + 1.0,
            expected_start_measures.at(i), 0.05);
    }
}

BOOST_AUTO_TEST_SUITE_END()
