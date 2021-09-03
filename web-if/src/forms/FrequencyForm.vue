<template>
  <v-container>
    <h2 class="py-4">Photo Frequency</h2>
    <v-alert v-model="alert_error" border="top" color="red lighten-2" dark>{{ alert_error_text }}</v-alert>
    <v-alert v-model="alert_success" border="top" color="green lighten-2" show dismissible dark>{{ alert_success_text }}</v-alert>
    <v-form v-if="loaded" ref="form">
        <v-card class="pt-6" color="grey lighten-3" flat>
          <v-container>
            <v-row>
              <v-col cols="12">
                <h3 ref="radio" class="text-h5">Current State</h3>
                <div class="my-4">
                  The camera is operating in <b>{{ modeName(current_settings.mode) }} mode</b>{{ ((current_settings.end != null) && (requested_settings == null))? " until " + new Date(current_settings.end).toLocaleString() : "" }}.<br/>
                  <span v-if="requested_settings != null">Switch to <b>{{ modeName(requested_settings.mode) }} mode</b>{{ requested_settings.mode_end != null ? " until " + new Date(requested_settings.mode_end).toLocaleString() : "" }} is scheduled.<br/>Waiting for camera contact.</span></div>
                <v-btn v-if="canCancelRequest()" @click="apply_regular_mode()" class="mr-4 my-4 primary">Cancel Mode</v-btn>
              </v-col>
              <v-col cols="12">
                <h3 ref="radio" class="text-h5">Temporarily Increase Frequency</h3>
                <div class="mb-3 subtitle-2 red--text">Warning: The camera is battery powered! More photos mean shorter battery life. Use this function conservatively.</div>
                <v-radio-group row v-model="mode">
                    <v-radio label="Take a photo every 30 minutes (fast mode)" value="fast" />
                    <v-radio label="Take a photo every 10 minutes (express mode)" value="express" />
                </v-radio-group>
                <v-select v-if="mode=='fast'" item-value="value" item-text="key" v-model="duration" :items="duration_fast_options" label="For how long"></v-select>
                <v-select v-if="mode=='express'" item-value="value" item-text="key" v-model="duration" :items="duration_express_options" label="For how long"></v-select>
                <v-btn @click="apply()"  :disabled="!applicable()" class="mr-4 my-4 primary">Increase frequency</v-btn>
                <v-btn @click="mode=null" class="mr-4 my-4 primary">Discard</v-btn>
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
import { AUTH_LOGOUT } from "../store/actions/auth";

export default {
  name: 'FrequencyForm',
  components: {
  },
  data: () => ({
    alert_success: false,
    alert_success_text: null,
    alert_error: false,
    alert_error_text: null,
    current_settings: null,
    requested_settings: null,
    duration_fast_options: [ { key: '3 hours', value: 1 }, { key: '6 hours', value: 2 },  { key: 'until the end of the day', value: 3 }],
    duration_express_options: [ { key: '1 hour', value: 4 }, {key: '2 hours', value: 5 }, { key: '3 hours', value: 6 }],
    loaded: false,
    duration: null,
    mode: null,
    tab: null,
    updateInterval: null
  }),
  methods: {
    modeName(str) {
      switch (str) {
        case 0: return "regular"
        case 1: return "fast"
        case 2: return "express"
      }
    },
    hasChanged(parameter) {
      if (this.parameters[parameter]) {
        return this.parameters[parameter].activeValue != this.parameters[parameter].modifiedValue
      } else return false
    },
    canCancelRequest() {
      // a mode other than regular is requested
      if (this.requested_settings && this.requested_settings.mode > 0) return true
      // we want to go to regular mode and cam is in fast or express mode
      if (this.current_settings && this.current_settings.mode > 0) return true
      return false
    },
    applicable() {
      if (this.mode == "fast") return ((this.duration >= 1) && (this.duration <= 3))
      if (this.mode == "express") return ((this.duration >= 4) && (this.duration <= 6))
      return false
    },
    apply() {
      if (!this.applicable()) return

      // post to backend
      axios.post(process.env['VUE_APP_BACKENDURL']+'/set_frequency', {
          token: store.getters.currentToken,
          mode_identifier: this.duration
      })
      .then((response) => {
        this.current_settings = response.data.current
        this.requested_settings = response.data.requested
        this.loaded = true
        this.alert_success_text = "Change successfully scheduled"
        this.alert_success = true
        this.alert_error = false
      }, (error) => {
        if (error.response.status == 401) {
          // session expired
          this.$store.dispatch(AUTH_LOGOUT).then(() => {
            this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
          }).catch(() => { /* TODO */ });
        }
        this.loaded = false
        this.alert_error_text = error.message
        this.alert_error = true
        this.alert_success = false
      });

    },
    apply_regular_mode() {
      // post to backend
      axios.post(process.env['VUE_APP_BACKENDURL']+'/set_frequency', {
          token: store.getters.currentToken,
          mode_identifier: 0
      })
      .then((response) => {
        this.current_settings = response.data.current
        this.requested_settings = response.data.requested
        this.loaded = true
        this.alert_success_text = "Change successfully scheduled"
        this.alert_success = true
        this.alert_error = false
      }, (error) => {
        if (error.response.status == 401) {
          // session expired
          this.$store.dispatch(AUTH_LOGOUT).then(() => {
            this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
          }).catch(() => { /* TODO */ });
        }
        this.loaded = false
        this.alert_error_text = error.message
        this.alert_error = true
        this.alert_success = false
      });
    },
    reload_current_mode() {
      axios.post(process.env['VUE_APP_BACKENDURL']+'/get_frequency', {
        token: store.getters.currentToken,
      })
      .then((response) => {
          this.current_settings = response.data.current
          this.requested_settings = response.data.requested
          this.loaded = true
          this.alert_success = false
          this.alert_error = false
        }, (error) => {
          if (error.response.status == 401) {
            // session expired
            this.$store.dispatch(AUTH_LOGOUT).then(() => {
              this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
            }).catch(() => { /* TODO */ });
          }
          this.loaded = false
          this.alert_error_text = error.message
          this.alert_error = true
          this.alert_success = false
      });
    }
  },
  created() {
    this.updateInterval = setInterval(function() { this.reload_current_mode() }.bind(this), 30000)
  },
  destroyed() {
    clearInterval(this.updateInterval)
  },
  mounted () {
    this.reload_current_mode();
  }
};
</script>

