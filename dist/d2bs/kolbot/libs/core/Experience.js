/* eslint-disable max-len */
/**
*  @filename    Experience.js
*  @author      kolton
*  @desc        Experience library
*
*/

const Experience = {
  /**
   * @todo combine this and nextExp into key-value pairs 1-99
   * Experience[me.charlvl].total and Experience[me.charlvl].next
   */
  totalExp: [
    0, 0, 500, 1500, 3750, 7875, 14175, 22680, 32886, 44396, 57715, 72144, 90180, 112725,
    140906, 176132, 220165, 275207, 344008, 430010, 537513, 671891, 839864, 1049830, 1312287,
    1640359, 2050449, 2563061, 3203826, 3902260, 4663553, 5493363, 6397855, 7383752, 8458379,
    9629723, 10906488, 12298162, 13815086, 15468534, 17270791, 19235252, 21376515, 23710491,
    26254525, 29027522, 32050088, 35344686, 38935798, 42850109, 47116709, 51767302, 56836449,
    62361819, 68384473, 74949165, 82104680, 89904191, 98405658, 107672256, 117772849, 128782495,
    140783010, 153863570, 168121381, 183662396, 200602101, 219066380, 239192444, 261129853,
    285041630, 311105466, 339515048, 370481492, 404234916, 441026148, 481128591, 524840254,
    572485967, 624419793, 681027665, 742730244, 809986056, 883294891, 963201521, 1050299747,
    1145236814, 1248718217, 1361512946, 1484459201, 1618470619, 1764543065, 1923762030,
    2097310703, 2286478756, 2492671933, 2717422497, 2962400612, 3229426756, 3520485254, 0, 0
  ],
  nextExp: [
    0, 500, 1000, 2250, 4125, 6300, 8505, 10206, 11510, 13319, 14429, 18036, 22545, 28181,
    35226, 44033, 55042, 68801, 86002, 107503, 134378, 167973, 209966, 262457, 328072, 410090,
    512612, 640765, 698434, 761293, 829810, 904492, 985897, 1074627, 1171344, 1276765, 1391674,
    1516924, 1653448, 1802257, 1964461, 2141263, 2333976, 2544034, 2772997, 3022566, 3294598,
    3591112, 3914311, 4266600, 4650593, 5069147, 5525370, 6022654, 6564692, 7155515, 7799511,
    8501467, 9266598, 10100593, 11009646, 12000515, 13080560, 14257811, 15541015, 16939705,
    18464279, 20126064, 21937409, 23911777, 26063836, 28409582, 30966444, 33753424, 36791232,
    40102443, 43711663, 47645713, 51933826, 56607872, 61702579, 67255812, 73308835, 79906630,
    87098226, 94937067, 103481403, 112794729, 122946255, 134011418, 146072446, 159218965, 173548673,
    189168053, 206193177, 224750564, 244978115, 267026144, 291058498, 0, 0
  ],
  expCurve: [13, 16, 110, 159, 207, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 225, 174, 92, 38, 5],
  expPenalty: [1024, 976, 928, 880, 832, 784, 736, 688, 640, 592, 544, 496, 448, 400, 352, 304, 256, 192, 144, 108, 81, 61, 46, 35, 26, 20, 15, 11, 8, 6, 5],
  monsterExp: [
    [1, 1, 1], [30, 78, 117], [40, 104, 156], [50, 131, 197], [60, 156, 234], [70, 182, 273], [80, 207, 311], [90, 234, 351], [100, 260, 390], [110, 285, 428], [120, 312, 468],
    [130, 338, 507], [140, 363, 545], [154, 401, 602], [169, 440, 660], [186, 482, 723], [205, 533, 800], [225, 584, 876], [248, 644, 966], [273, 708, 1062], [300, 779, 1169],
    [330, 857, 1286], [363, 942, 1413], [399, 1035, 1553], [439, 1139, 1709], [470, 1220, 1830], [503, 1305, 1958], [538, 1397, 2096], [576, 1494, 2241], [616, 1598, 2397],
    [659, 1709, 2564], [706, 1832, 2748], [755, 1958, 2937], [808, 2097, 3146], [864, 2241, 3362], [925, 2399, 3599], [990, 2568, 3852], [1059, 2745, 4118], [1133, 2939, 4409],
    [1212, 3144, 4716], [1297, 3365, 5048], [1388, 3600, 5400], [1485, 3852, 5778], [1589, 4121, 6182], [1693, 4409, 6614], [1797, 4718, 7077], [1901, 5051, 7577],
    [2005, 5402, 8103], [2109, 5783, 8675], [2213, 6186, 9279], [2317, 6618, 9927], [2421, 7080, 10620], [2525, 7506, 11259], [2629, 7956, 11934], [2733, 8435, 12653],
    [2837, 8942, 13413], [2941, 9477, 14216], [3045, 10044, 15066], [3149, 10647, 15971], [3253, 11286, 16929], [3357, 11964, 17946], [3461, 12680, 19020],
    [3565, 13442, 20163], [3669, 14249, 21374], [3773, 15104, 22656], [3877, 16010, 24015], [3981, 16916, 25374], [4085, 17822, 26733], [4189, 18728, 28092],
    [4293, 19634, 29451], [4397, 20540, 30810], [4501, 21446, 32169], [4605, 22352, 33528], [4709, 23258, 34887], [4813, 24164, 36246], [4917, 25070, 37605],
    [5021, 25976, 38964], [5125, 26882, 40323], [5229, 27788, 41682], [5333, 28694, 43041], [5437, 29600, 44400], [5541, 30506, 45759], [5645, 31412, 47118],
    [5749, 32318, 48477], [5853, 33224, 49836], [5957, 34130, 51195], [6061, 35036, 52554], [6165, 35942, 53913], [6269, 36848, 55272], [6373, 37754, 56631],
    [6477, 38660, 57990], [6581, 39566, 59349], [6685, 40472, 60708], [6789, 41378, 62067], [6893, 42284, 63426], [6997, 43190, 64785], [7101, 44096, 66144],
    [7205, 45002, 67503], [7309, 45908, 68862], [7413, 46814, 70221], [7517, 47720, 71580], [7621, 48626, 72939], [7725, 49532, 74298], [7829, 50438, 75657],
    [7933, 51344, 77016], [8037, 52250, 78375], [8141, 53156, 79734], [8245, 54062, 81093], [8349, 54968, 82452], [8453, 55874, 83811], [160000, 160000, 160000]
  ],
  /**
   * Percent progress into the current level. Format: xx.xx%
   */
  progress: function () {
    return me.getStat(sdk.stats.Level) === 99 ? 0 : (((me.getStat(sdk.stats.Experience) - this.totalExp[me.getStat(sdk.stats.Level)]) / this.nextExp[me.getStat(sdk.stats.Level)]) * 100).toFixed(2);
  },

  /**
   * Total experience gained in current run
   */
  gain: function () {
    return (me.getStat(sdk.stats.Experience) - DataFile.getStats().experience);
  },

  /**
   * Percent experience gained in current run
   */
  gainPercent: function () {
    return me.getStat(sdk.stats.Level) === 99 ? 0 : (this.gain() * 100 / this.nextExp[me.getStat(sdk.stats.Level)]).toFixed(6);
  },

  /**
   * Runs until next level
   */
  runsToLevel: function () {
    return Math.round(((100 - this.progress()) / 100) * this.nextExp[me.getStat(sdk.stats.Level)] / this.gain());
  },

  /**
   * Total runs needed for next level (not counting current progress)
   */
  totalRunsToLevel: function () {
    return Math.round(this.nextExp[me.getStat(sdk.stats.Level)] / this.gain());
  },

  /**
   * Total time till next level
   */
  timeToLevel: function () {
    let tTLrawSeconds = (Math.floor((getTickCount() - me.gamestarttime) / 1000)).toString();
    let tTLrawtimeToLevel = this.runsToLevel() * tTLrawSeconds;
    let tTLDays = Math.floor(tTLrawtimeToLevel / 86400);
    let tTLHours = Math.floor((tTLrawtimeToLevel % 86400) / 3600);
    let tTLMinutes = Math.floor(((tTLrawtimeToLevel % 86400) % 3600) / 60);
    //let tTLSeconds = ((tTLrawtimeToLevel % 86400) % 3600) % 60;

    //return tDays + "d " + tTLHours + "h " + tTLMinutes + "m " + tTLSeconds + "s";
    //return tTLDays + "d " + tTLHours + "h " + tTLMinutes + "m";
    return (tTLDays ? tTLDays + " d " : "") + (tTLHours ? tTLHours + " h " : "") + (tTLMinutes ? tTLMinutes + " m" : "");
  },

  /**
   * Get Game Time
   */
  getGameTime: function () {
    let rawMinutes = Math.floor((getTickCount() - me.gamestarttime) / 60000).toString();
    let rawSeconds = (Math.floor((getTickCount() - me.gamestarttime) / 1000) % 60).toString();

    rawMinutes <= 9 && (rawMinutes = "0" + rawMinutes);
    rawSeconds <= 9 && (rawSeconds = "0" + rawSeconds);

    return " (" + rawMinutes + ":" + rawSeconds + ")";
  },

  /**
   * Log to manager
   */
  log: function () {
    let gain = this.gain();
    let progress = this.progress();
    let runsToLevel = this.runsToLevel();
    let getGameTime = this.getGameTime();
    let string = "[Game: " + me.gamename + (me.gamepassword ? "//" + me.gamepassword : "") + getGameTime + "] [Level: " + me.getStat(sdk.stats.Level) + " (" + progress + "%)] [XP: " + gain + "] [Games ETA: " + runsToLevel + "]";

    if (gain) {
      D2Bot.printToConsole(string, sdk.colors.D2Bot.Blue);

      if (me.getStat(sdk.stats.Level) > DataFile.getStats().level) {
        D2Bot.printToConsole("Congrats! You gained a level. Current level:" + me.getStat(sdk.stats.Level), sdk.colors.D2Bot.Green);
      }
    }
  }
};
