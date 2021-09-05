<template>
  <v-main>
    <v-container fluid fill-height>
      <v-layout align-center justify-center>
          <v-flex xs12 sm8 md4>
            <v-card class="elevation-12">
                <v-toolbar dark color="primary">
                  <v-toolbar-title>Webcam Login</v-toolbar-title>
                </v-toolbar>
                <v-card-text>
                  <v-form>
                      <v-text-field
                        prepend-icon="mdi-account"
                        label="User"
                        ref="user"
                        type="text"
                        v-model="loginUser"
                        v-on:keyup.enter="login"
                      ></v-text-field>
                      <v-text-field
                        id="password"
                        prepend-icon="mdi-lock"
                        label="Password"
                        type="password"
                        v-model="loginPassword"
                        v-on:keyup.enter="login"
                      ></v-text-field>
                  </v-form>
                  <div v-if="accessDenied" class="red darken-2 text-center px-4 py-1 mt-3">
                    <span class="white--text">Access Denied</span>
                  </div>
                  <div v-if="message" class="red darken-2 text-center px-4 py-1 mt-3">
                    <span class="white--text">{{ message }}</span>
                  </div>
                </v-card-text>
                <v-card-actions>
                  <v-spacer></v-spacer>
                  <v-btn @click="login" type="submit" color="primary">Login</v-btn>
                </v-card-actions>
            </v-card>
          </v-flex>
      </v-layout>
    </v-container>
    <v-layout align-center justify-center>  <span class="font-weight-thin caption" >v0.4</span></v-layout>
  </v-main>
</template>

<script>
  import { AUTH_REQUEST } from "../store/actions/auth"

  export default {
    name: "LoginForm",
    props: ['message'],
    methods: {
      login() {
        this.$store.dispatch(AUTH_REQUEST, { user: this.loginUser, password: this.loginPassword }).then(() => {
          this.accessDenied = false
          this.$router.push('/photos')
        }).catch(() => {
          this.accessDenied = true
          this.message = null
      });
    }
  },
  mounted() {
    this.$refs.user.focus()
  },
  data: () => ({
    loginUser: null,
    loginPassword: null,
    dialog: null,
    accessDenied: false
  }),
  }
</script>