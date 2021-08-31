<template>
  <v-container>
    <h2 class="py-4">Photo Frequency</h2>
    <v-alert v-if="error != null" border="top" color="red lighten-2" dark>{{ error }}</v-alert>
    <v-form v-if="loaded" ref="form">
        <v-card class="pt-6" color="grey lighten-3" flat>
          <v-container>
            <v-row>
              <v-col cols="12">
                <h3 ref="radio" class="text-h5">Current State</h3>
                <div class="my-4">The camera is operating in regular mode. A photo is take every X minutes.<br/>Fast Mode was activated at 1:30pm, 12/11/2021</div>
                <v-btn class="mr-4 my-4 primary">Go back to regular mode</v-btn>
              </v-col>
              <v-col cols="12">
                <h3 ref="radio" class="text-h5">Temporarily Increase Frequency</h3>
                <div class="mb-3 subtitle-2 red--text">Warning: The camera is battery powered! More photos mean shorter battery life. Use this function conservatively.</div>
                <v-radio-group row>
                    <v-radio label="Take a photo every 30 minutes (fast mode)" value="radio-2"/>
                    <v-radio label="Take a photo every 10 minutes (express mode)" value="radio-1"/>
                </v-radio-group>
                <v-select :items="duration_fast" label="For how long"></v-select>
                <v-btn class="mr-4 my-4 primary">Increase frequency</v-btn>
              </v-col>
            </v-row>
          </v-container>
        </v-card>
  </v-form>
  </v-container>
</template>

<script>
import axios from 'axios';
import store from "../store";

export default {
  name: 'FrequencyForm',
  components: {
  },
  data: () => ({
    duration_fast: ['3 hours', '6 hours', 'until the end of the day'],
    duration_express: ['1 hour', '2 hours', '3 hours'],
    error: null,
    loaded: false,
    pinRules: [
        value => (parseInt(value) >= 0 | value == '') || 'Must be an integer with at least five digits or empty',
      ],
    tab: null,
    whitebalance_modes: [
      { text: 'Auto', value: 0 },
      { text: 'Sunny', value: 1 },
      { text: 'Cloudy', value: 2 },
      { text: 'Office', value: 3 },
      { text: 'Home', value: 4 }
    ],
    apnauth_modes: [
      { text: 'None', value: 0 },
      { text: 'PAP', value: 1 },
      { text: 'CHAP', value: 2 }
    ],
    parameters: {
      "cam_quality": { activeValue: 0, modifiedValue: 0, defaultValue: 10 },
      "cam_auto_exposure": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_light_mode": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_color_saturation": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_brightness": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_contrast": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_hue": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_sharpness": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cell_pin": { activeValue: "", modifiedValue: "", defaultValue: "1000" },
      "cell_apn": { activeValue: "", modifiedValue: "", defaultValue: "internet" },
      "cell_apn_user": { activeValue: "", modifiedValue: "", defaultValue: "" },
      "cell_apn_pass": { activeValue: "", modifiedValue: "", defaultValue: "" },
      "cell_apn_auth": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cell_remote_url": { activeValue: "", modifiedValue: "", defaultValue: "http://wwv-schwarzwald.de/webcam/api/upload" },
      "sys_secs_between_photos": { activeValue: 0, modifiedValue: 0, defaultValue: 3600 },
    },

  }),
  methods: {
    hasChanged(parameter) {
      if (this.parameters[parameter]) {
        return this.parameters[parameter].activeValue != this.parameters[parameter].modifiedValue
      } else return false
    },
    apply() {
      // create array
      var newConfig = {}
      for (var key in this.parameters) {
        if (this.hasChanged(key)) {
          newConfig[key] = this.parameters[key].modifiedValue
        }
      }
      console.log(JSON.stringify(newConfig))
      // post to backend
      axios.post(process.env['VUE_APP_BACKENDURL']+'/set_config', JSON.stringify(newConfig), {
        auth: {
            username: store.getters.currentUser,
            password: store.getters.currentPassword
        }})
      .then((response) => {
        console.log(response);
      }, (error) => {
        console.log(error);
      });
      this.$router.push('/photos')
    },
    discard() {
      this.$router.push('/photos')
    }
  },
  mounted () {
    axios.get(process.env['VUE_APP_BACKENDURL']+'/get_config', {
      auth: {
          username: store.getters.currentUser,
          password: store.getters.currentPassword
      }})
    .then((response) => {
      for (var key in this.parameters) {
        if (response.data.active && response.data.active[key]) {
          this.parameters[key].activeValue = response.data.active[key]
        } else {
          this.parameters[key].activeValue = this.parameters[key].defaultValue
        }
        if (response.data.modified && response.data.modified[key]) {
          this.parameters[key].modifiedValue = response.data.modified[key]
        } else {
          this.parameters[key].modifiedValue = this.parameters[key].activeValue
        }
      }
      this.loaded = true
    }, (error) => {
      this.error = "Unable to load data: " + error.message
    });
}
};
</script>

