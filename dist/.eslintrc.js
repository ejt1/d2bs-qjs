/* eslint-disable linebreak-style */
// Standard Formatting rules for Kolbot scripts
// Compatible with ESLint plugin

module.exports = {
  "root": true,
  "extends": "eslint:recommended",
  "parserOptions": {
    "ecmaFeatures": {
      "impliedStrict": true,
    }
  },
  "env": {
    "es6": true,
  },
  "globals": {
    // The following globals are defined within D2BS, or are actually defined in the source code
    "include": true,
    "print": true,
    "me": true,
    "td": true,
    "getTickCount": true,
    "delay": true,
    "getParty": true,
    "takeScreenshot": true,
    "getUnit": true,
    "quit": true,
    "clickMap": true,
    "getBaseStat": true,
    "clickItem": true,
    "getCursorType": true,
    "getPresetUnits": true,
    "getDistance": true,
    "copyUnit": true,
    "getRoom": true,
    "getLocaleString": true,
    "scriptBroadcast": true,
    "isIncluded": true,
    "showConsole": true,
    "getInteractedNPC": true,
    "getDialogLines": true,
    "getUIFlag": true,
    "sendPacket": true,
    "getPacket": true,
    "getPath": true,
    "rand": true,
    "PresetUnit": true,
    "getPresetUnit": true,
    "getArea": true,
    "getWaypoint": true,
    "getScript": true,
    "Room": true,
    "say": true,
    "load": true,
    "addEventListener": true,
    "getMercHP": true,
    "checkCollision": true,
    "gold": true,
    "getLocation": true,
    "login": true,
    "sendCopyData": true,
    "getControl": true,
    "debugLog": true,
    "getCollision": true,
    "transmute": true,
    "submitItem": true,
    "createGame": true,
    "joinGame": true,
    "Line": true,
    "removeEventListener": true,
    "Unit": true,
    "Party": true,
    "UtilitySystem": true,
    "moveNPC": true,
    "getPlayerFlag": true,
    "clickParty": true,
    "dopen": true,
    "Items": true,
    "Text": true,
    "File": true,
    "js_strict": true,
    "handler": true,
    "sendKey": true,
    "md5": true,
    "module": true,
    "require": true,
    "Box": true,
    "Frame": true,
    "revealLevel": true,
    "hideConsole": true,
  },
  "rules": {
    // enable additional rules
    "indent": ["warn", 2],
    "linebreak-style": ["off", "windows"],
    "semi": ["error", "always"],
    "comma-spacing": ["error", { "before": false, "after": true }],
    "keyword-spacing": ["error", { "before": true, "after": true }],
    "object-curly-spacing": ["error", "always"],
    "brace-style": ["error", "1tbs", { "allowSingleLine": true }],
    "space-infix-ops": "error",
    "space-unary-ops": ["error", { "words": true, "nonwords": false }],
    "arrow-spacing": "error",
    "arrow-body-style": ["error", "as-needed"],
    "space-before-blocks": "error",
    "key-spacing": ["error", { "mode": "strict", "beforeColon": false, "afterColon": true }],
    "no-mixed-spaces-and-tabs": "error",
    "no-trailing-spaces": ["warn", { "ignoreComments": true, "skipBlankLines": true }],
    "no-whitespace-before-property": "error",
    "comma-style": ["error", "last"],
    "eol-last": ["error", "always"],
    "block-scoped-var": "error",
    "no-var": "warn",
    "curly": ["error", "multi-line"],
    "dot-notation": "warn",
    "eqeqeq": ["error", "smart"],
    "no-caller": "error",
    "no-floating-decimal": "error",
    "no-multi-spaces": ["error", { "ignoreEOLComments": true }],
    "no-self-compare": "error",
    "no-case-declarations": "off",
    "no-with": "error",
    "no-shadow": "off",
    "no-use-before-define": "off",
    "no-prototype-builtins": "off",
    "quotes": ["warn", "double", { "avoidEscape": true }],
    "no-constant-condition": ["error", { "checkLoops": false }],
    "no-extra-label": "error",
    //"no-labels": ["error", {"allowLoop": true}], // in the future no loops ;)
    "no-unused-vars": ["warn", { "vars": "local" }],
    "no-fallthrough": ["error", { "commentPattern": "break[\\s\\w]*omitted" }],
    "no-undef": ["off", "always"],
    "no-extra-boolean-cast": ["off", "always"],
    "max-len": ["warn", { "code": 120, "ignoreComments": true, "ignoreUrls": true }],
  }
};
