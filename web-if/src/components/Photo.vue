<template>
  <v-container fluid v-if="value">
    <v-toolbar color="indigo" dark flat dense>
      <v-tabs v-model="tab">
          <v-spacer/>
          <v-tab key="photo">Photo</v-tab>
          <v-tab key="telemetry">Telemetry</v-tab>
          <v-spacer/>
      </v-tabs>
    </v-toolbar>
    <v-tabs-items  v-model="tab">
      <v-tab-item key="photo">
        <v-card color="grey lighten-3" flat>
          <v-card-text>
            <p class="text-center my-2">{{ new Date(value.timestamp * 1000).toLocaleDateString("en-US",  { timeZone: 'Europe/Berlin' }) }} - {{ new Date(value.timestamp * 1000).toLocaleTimeString("en-US",  { timeZone: 'Europe/Berlin' }) }}</p>
            <v-img v-if="photoData != null" class="my-2" :src="photoData" />
          </v-card-text>
        </v-card>
      </v-tab-item>
      <v-tab-item key="telemetry">
        <v-card color="grey lighten-3" flat>
          <v-card-text>
            <p class="text-center my-2">{{ new Date(value.timestamp * 1000).toLocaleDateString("en-US",  { timeZone: 'Europe/Berlin' }) }} - {{ new Date(value.timestamp * 1000).toLocaleTimeString("en-US",  { timeZone: 'Europe/Berlin' }) }}</p>
            <h3 class="my-3">Power Supply</h3>
            <v-simple-table v-if="value != null" dense color="grey lighten-3">
              <thead>
                <tr><th class="text-left">Measure</th><th class="text-left">Value</th><th class="text-left">Unit</th></tr>
              </thead>
              <tbody>
                <tr><td>Battery Voltage during Boot Process</td><td>{{ Number(value.telemetry.vsupply_startup).toFixed(2) }}</td><td>V</td></tr>
                <tr><td>Battery Current during Boot Process</td><td>{{ (Number(value.telemetry.current_startup) * 1000).toFixed(1) }} </td><td>mA</td></tr>
                <tr><td>Min. Battery Voltage during GPRS Connection Negotiation</td><td>{{ Number(value.telemetry.voltage_min_cell_negotiation).toFixed(2) }}</td><td>V</td></tr>
                <tr><td>Avg. Battery Current during GPRS Connection Negotiation</td><td>{{ (Number(value.telemetry.current_avg_cell_negotiation)* 1000).toFixed(1) }}</td><td>mA</td></tr>
                <tr><td>Supply Voltage for GSM module</td><td> {{ (Number(value.telemetry.gsm_voltage) / 1000).toFixed(2) }}</td><td>V</td></tr>
              </tbody>
            </v-simple-table>
            <h3 class="my-3">GSM</h3>
            <v-simple-table v-if="value != null" dense color="grey lighten-3">
              <thead>
                <tr><th class="text-left">Measure</th><th class="text-left">Value</th><th class="text-left">Unit</th></tr>
              </thead>
              <tbody>
                <tr><td>Broadcast Control Channel (BCCH)</td><td colspan="2">{{ value.telemetry.gsm_bcch }}</td></tr>
                <tr><td>Base Station Identity Code (BSIC)</td><td colspan="2">{{ value.telemetry.gsm_bsic }}</td></tr>
                <tr><td>Base Station Location (courtesy of nobbi.com)</td><td v-if="cell_info_from_api" colspan="2">{{ cell_info_from_api.site.description }}<v-btn small icon color="blue" :href="osmurl" target="_blank"><v-icon>mdi-map-marker</v-icon></v-btn></td></tr>
                <tr><td>Cell ID</td><td colspan="2">{{ value.telemetry.gsm_cellid }}</td></tr>
                <tr><td>Location Area (LAC)</td><td colspan="2">{{ value.telemetry.gsm_lac }}</td></tr>
                <tr><td>Country Code (MCC)</td><td colspan="2">{{ value.telemetry.gsm_mcc }}</td></tr>
                <tr><td>Network Code (MNC)</td><td colspan="2">{{ value.telemetry.gsm_mnc }}</td></tr>
                <tr><td>Network Operator</td><td v-if="cell_info_from_api" colspan="2">{{ cell_info_from_api.provider.country }} / {{ cell_info_from_api.provider.operator }} ({{ cell_info_from_api.provider.brand }})</td></tr>
                <tr><td>Receive Level (RXL)</td><td>{{ rxlValToDbM[value.telemetry.gsm_rxl] }}</td><td>dBm</td></tr>
                <tr><td>Receive Level Access Minimum (RLA)</td><td>{{ rxlValToDbM[value.telemetry.gsm_rla] }}</td><td>dBm</td></tr>
                <tr><td>Receive Quality (RXQ)</td><td colspan="2"><span :style="rxqColor[value.telemetry.gsm_rxq]">{{ value.telemetry.gsm_rxq }} / 7</span></td></tr>
                <tr><td>Timing Advance (TA)</td><td colspan="2">{{ value.telemetry.gsm_ta }} / 63</td></tr>
                <tr><td>Distance to Base Station</td><td>{{ timingAdvanceToString(value.telemetry.gsm_ta) }}</td><td>m</td></tr>
                <tr><td>Transmit Power Maximum CCCH (TXP)</td><td>{{ rxlValToDbM[value.telemetry.gsm_txp] }}</td><td>dBm</td></tr>
              </tbody>
            </v-simple-table>
          </v-card-text>
        </v-card>
      </v-tab-item>
    </v-tabs-items>
  </v-container>
</template>
<script>
    import axios from 'axios';
    export default {
        name: 'Photo',
        components: {
        },
        props: [ 'value' ],
    data: () => ({
        rxqColor: {
          0: "color: green",
          1: "color: green",
          2: "color: green",
          3: "color: orange",
          4: "color: orange",
          5: "color: red",
          6: "color: red",
          7: "color: red"
        },
        rxlValToDbM: {
          0: "< -110",
          1: "-110 to -109",
          2: "-109 to -108",
          3: "-108 to -107",
          4: "-107 to -106",
          5: "-106 to -105",
          6: "-105 to -104",
          7: "-104 to -103",
          8: "-103 to -102",
          9: "-102 to -101",
          10: "-101 to -100",
          11: "-100 to -99",
          12: "-99 to -98",
          13: "-98 to -97",
          14: "-97 to -96",
          15: "-96 to -95",
          16: "-95 to -94",
          17: "-94 to -93",
          18: "-93 to -92",
          19: "-92 to -91",
          20: "-91 to -90",
          21: "-90 to -89",
          22: "-89 to -88",
          23: "-88 to -87",
          24: "-87 to -86",
          25: "-86 to -85",
          26: "-85 to -84",
          27: "-84 to -83",
          28: "-83 to -82",
          29: "-82 to -81",
          30: "-81 to -80",
          31: "-80 to -79",
          32: "-79 to -78",
          33: "-78 to -77",
          34: "-77 to -76",
          35: "-76 to -75",
          36: "-75 to -74",
          37: "-74 to -73",
          38: "-73 to -72",
          39: "-72 to -71",
          40: "-71 to -70",
          41: "-70 to -69",
          42: "-69 to -68",
          43: "-68 to -67",
          44: "-67 to -66",
          45: "-66 to -65",
          46: "-65 to -64",
          47: "-64 to -63",
          48: "-63 to -62",
          49: "-62 to -61",
          50: "-61 to -60",
          51: "-60 to -59",
          52: "-59 to -58",
          53: "-58 to -57",
          54: "-57 to -56",
          55: "-56 to -55",
          56: "-55 to -54",
          57: "-54 to -53",
          58: "-53 to -52",
          59: "-52 to -51",
          60: "-51 to -50",
          61: "-50 to -49",
          62: "-49 to -48",
          63: "> -48"
      },
      cell_info_from_api: null,
      tab: null,
      photoData: null
    }),
    watch: {
      value: function(val) {
        if(val) {
          this.loadCellInfoFromApi(val.telemetry.gsm_mcc, val.telemetry.gsm_mnc, val.telemetry.gsm_cellid)
          this.loadPhoto(val.id)
        } else {
         this.cell_info_from_api = null
        }
      }
    },
    methods: {
      loadPhoto: function(id) {
        this.photoData = null
        let authData = JSON.parse(localStorage.authentication)
        if (authData && authData.authenticated) {
          let queryStr = 'id='+id
          axios.get('https://wwv-schwarzwald.de/webcam/api/photo?'+queryStr,
            {  responseType: "arraybuffer" ,
              auth: {
                username: authData.user,
                password: authData.password
          }}).then(response => {
            let base64String = btoa(
              String.fromCharCode.apply(null, new Uint8Array(response.data))
            );
            this.photoData = "data:image/jpg;base64," + base64String;
          });
        }
      },
      loadCellInfoFromApi: function(mcc, mnc, cid) {
        this.cell_info_from_api = null
        let authData = JSON.parse(localStorage.authentication)
        if (authData && authData.authenticated) {
          let queryStr = 'mcc='+mcc+'&mnc='+mnc+'&cid='+cid;
          axios.get('https://wwv-schwarzwald.de/webcam/api/cell_info?'+queryStr,
            { auth: {
                username: authData.user,
                password: authData.password
          }}).then(response => {
            this.cell_info_from_api = response.data
          });
        }
      },
      timingAdvanceToString: function(val) {
        if (val == 0xff) {
          return "Unknown"
        } else {
          const dist = Number(this.value.telemetry.gsm_ta) * 550;
          return dist + " to " + (dist+449)
        }
      }
    },
    mounted() {
      if (this.value) {
        this.loadCellInfoFromApi(this.value.telemetry.gsm_mcc, this.value.telemetry.gsm_mnc, this.value.telemetry.gsm_cellid)
      } else {
        this.cell_info_from_api = null
      }
    },
    computed: {
      osmurl: function() {
        return "http://www.openstreetmap.org/?mlat="+this.cell_info_from_api.site.latitude+"&mlon="+this.cell_info_from_api.site.longitude+"&zoom=12"
      },
      photourl: function() {
        let authData = JSON.parse(localStorage.authentication)
        if (authData && authData.authenticated) {
          return "https://"+authData.user+":"+authData.password+"@wwv-schwarzwald.de/webcam/api/photo?id=" + this.value.id
        } else {
          return ""
        }
      },
      convertedTelemetryValues: function() {
        let values = []
        this.telemetryitems.forEach(element => {
          if (element.convert) {
            values.push({ name: element.name, value: element.convert(this.value.telemetry[element.value]), unit: element.unit})
          } else {
            values.push({ name: element.name, value: this.value.telemetry[element.value], unit: element.unit})
          }
        });
        return values
      }
    }
  }
</script>
