const HubCodes = [
    'api_v',
    'id',
    'client',
    'type',
    'update',
    'updates',
    'get',
    'last',
    'crc32',
    'discover',
    'name',
    'prefix',
    'icon',
    'PIN',
    'version',
    'max_upload',
    'http_transfer',
    'ota_type',
    'ws_port',
    'modules',
    'total',
    'used',
    'code',
    'OK',
    'ack',
    'info',
    'controls',
    'ui',
    'files',
    'notice',
    'alert',
    'push',
    'script',
    'refresh',
    'print',

    'error',
    'fs_err',
    'ota_next',
    'ota_done',
    'ota_err',
    'fetch_start',
    'fetch_chunk',
    'fetch_err',
    'upload_next',
    'upload_done',
    'upload_err',
    'ota_url_err',
    'ota_url_ok',

    'value',
    'maxlen',
    'rows',
    'regex',
    'align',
    'min',
    'max',
    'step',
    'dec',
    'unit',
    'font_size',
    'action',
    'nolabel',
    'suffix',
    'notab',
    'square',
    'disable',
    'hint',
    'len',
    'wwidth',
    'wheight',
    'data',
    'wtype',
    'keep',
    'exp',

    'plugin',
    'js',
    'css',
    'ui_file',
    'stream',
    'port',
    'canvas',
    'width',
    'height',
    'active',
    'html',
    'dummy',
    'menu',
    'gauge',
    'gauge_r',
    'gauge_l',
    'led',
    'log',
    'table',
    'image',
    'text',
    'display',
    'text_f',
    'label',
    'title',
    'dpad',
    'joy',
    'flags',
    'tabs',
    'switch_t',
    'switch_i',
    'button',
    'color',
    'select',
    'spinner',
    'slider',
    'datetime',
    'date',
    'time',
    'confirm',
    'prompt',
    'area',
    'pass',
    'input',
    'hook',
    'row',
    'col',
    'space',
    'platform',
    'map',
    'latlon',
    'location',
    'high_accuracy',
    'layer',
    'udp_port',
    'container',
    'rowcol',
    'spoiler',
    'http_port',
    'tags',
];

function decodeHubJson(data) {
        if (!data || !data.length) return null;

        data = data.trim()
            .replaceAll("#{", "{")
            .replaceAll("}#", "}")
            .replaceAll(/([^\\])\\([^\"\\nrt])/ig, "$1\\\\$2")
            .replaceAll(/\t/ig, "\\t")
            .replaceAll(/\n/ig, "\\n")
            .replaceAll(/\r/ig, "\\r");

        for (const code in HubCodes) {
            const re = new RegExp(`(#${Number(code).toString(16)})([:,\\]\\}])`, "ig");
            data = data.replaceAll(re, `"${HubCodes[code]}"$2`);
        }

    const re = /(#[0-9a-f][0-9a-f])([:,\]\}])/ig;
    if (data.match(re)) return null;
    else return data;
}


function parseResponse(res){
    if (res.length && res.startsWith('#{') && res.endsWith('}#')) {
        res = res.slice(2, -2);
        let chunks = res.split('}##{'); // возможны склеенные пакеты
        //for (let ch of chunks) {
            return JSON.parse(decodeHubJson( '{' + chunks[0] + '}' ))
        //}
    }

}

